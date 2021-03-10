/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_H_
#define NANOEM_EMAPP_MODEL_H_

#include "emapp/BoundingBox.h"
#include "emapp/Constants.h"
#include "emapp/IDrawable.h"
#include "emapp/IEffect.h"
#include "emapp/ITranslator.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/URI.h"
#include "emapp/model/Bone.h"
#include "emapp/model/Material.h"
#include "emapp/model/Morph.h"
#include "emapp/model/RigidBody.h"
#include "emapp/model/Vertex.h"

struct undo_command_t;
struct undo_stack_t;
typedef struct _Nanoem__Application__Command Nanoem__Application__Command;

namespace nanoem {

class Archiver;
class BaseApplicationService;
class Error;
class ICamera;
class ISeekableReader;
class IFileWriter;
class IWriter;
class IModelObjectSelection;
class Image;
class ImageLoader;
class ModelProgramBundle;
class Progress;
class Project;
class Vertex;

namespace internal {
class LineDrawer;
} /* namespace internal */

namespace model {
struct BindPose;
class ISkinDeformer;
} /* namespace model */

class Model NANOEM_DECL_SEALED : public IDrawable, private NonCopyable {
public:
    enum AxisType { kAxisTypeFirstEnum, kAxisCenter = kAxisTypeFirstEnum, kAxisX, kAxisY, kAxisZ, kAxisTypeMaxEnum };
    enum TransformCoordinateType {
        kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeGlobal = kTransformCoordinateTypeFirstEnum,
        kTransformCoordinateTypeLocal,
        kTransformCoordinateTypeMaxEnum
    };
    enum ResetType {
        kResetTypeFirstEnum,
        kResetTypeTranslationAxisX = kResetTypeFirstEnum,
        kResetTypeTranslationAxisY,
        kResetTypeTranslationAxisZ,
        kResetTypeOrientation,
        kResetTypeOrientationAngleX,
        kResetTypeOrientationAngleY,
        kResetTypeOrientationAngleZ,
        kResetTypeMaxEnum
    };
    typedef void (*UserDataDestructor)(void *userData, const Model *model);
    typedef tinystl::pair<void *, UserDataDestructor> UserData;
    BX_ALIGN_DECL_16(struct)
    VertexUnit
    {
        bx::simd128_t m_position;
        bx::simd128_t m_normal;
        bx::simd128_t m_texcoord;
        bx::simd128_t m_edge;
        bx::simd128_t m_uva[4];
        bx::simd128_t m_weights;
        bx::simd128_t m_indices;
        bx::simd128_t m_info; /* type,vertexIndex,edgeSize,padding */
        VertexUnit() NANOEM_DECL_NOEXCEPT;
        ~VertexUnit() NANOEM_DECL_NOEXCEPT;
        void setUVA(const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT;
        void performSkinning(nanoem_f32_t edgeSize, const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT;
        void setWeightColor(const model::Bone *bone, const model::Vertex *vertex) NANOEM_DECL_NOEXCEPT;
        void prepareSkinning(const model::Material::BoneIndexHashMap *indexHashMap, const model::Vertex *vertex)
            NANOEM_DECL_NOEXCEPT;
        static bx::simd128_t swizzleWeight(const model::Vertex *vertex, nanoem_rsize_t index) NANOEM_DECL_NOEXCEPT;
        static void performSkinningBdef1(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningBdef2(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningBdef4(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            nanoem_rsize_t i, bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningBdef4(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningQdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            nanoem_rsize_t i, bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningQdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningSdef(const model::Vertex *vertex, const bx::simd128_t op, const bx::simd128_t on,
            bx::simd128_t *p, bx::simd128_t *n) NANOEM_DECL_NOEXCEPT;
        static void performSkinningByType(const model::Vertex *vertex, bx::simd128_t *p, bx::simd128_t *n)
            NANOEM_DECL_NOEXCEPT;
    };
    struct ImportSetting {
        enum FileType {
            kFileTypeFirstEnum,
            kFileTypeNone = kFileTypeFirstEnum,
            kFileTypeWaveFrontObj,
            kFileTypeDirectX,
            kFileTypeMetasequoia,
            kFileTypeMaxEnum
        };
        ImportSetting(const URI &fileURI);
        ~ImportSetting() NANOEM_DECL_NOEXCEPT;
        const URI m_fileURI;
        String m_name[NANOEM_LANGUAGE_TYPE_MAX_ENUM];
        String m_comment[NANOEM_LANGUAGE_TYPE_MAX_ENUM];
        Matrix4x4 m_transform;
        FileType m_fileType;
    };
    struct ExportSetting {
        ExportSetting();
        ~ExportSetting() NANOEM_DECL_NOEXCEPT;
        Matrix4x4 m_transform;
    };

    static const Matrix4x4 kInitialWorldMatrix;
    static StringList loadableExtensions();
    static StringSet loadableExtensionsSet();
    static bool isLoadableExtension(const String &extension);
    static bool isLoadableExtension(const URI &fileURI);
    static void setStandardPipelineDescription(sg_pipeline_desc &desc);
    static void setEdgePipelineDescription(sg_pipeline_desc &desc);

    Model(Project *project, nanoem_u16_t handle);
    ~Model() NANOEM_DECL_NOEXCEPT;

    bool load(const nanoem_u8_t *bytes, size_t length, Error &error);
    bool load(const ByteArray &bytes, Error &error);
    bool load(const nanoem_u8_t *bytes, size_t length, const ImportSetting &setting, Error &error);
    bool load(const ByteArray &bytes, const ImportSetting &setting, Error &error);
    bool loadPose(const nanoem_u8_t *bytes, size_t length, Error &error);
    bool loadPose(const ByteArray &bytes, Error &error);
    bool loadArchive(const String &entryPoint, const Archiver &archiver, Error &error);
    bool loadArchive(const String &entryPoint, ISeekableReader *reader, Error &error);
    bool loadArchive(ISeekableReader *reader, Progress &progress, Error &error);
    bool save(IWriter *writer, Error &error) const;
    bool save(ByteArray &bytes, Error &error) const;
    bool save(IWriter *writer, const ExportSetting &setting, Error &error) const;
    bool save(ByteArray &bytes, const ExportSetting &setting, Error &error) const;
    bool savePose(IWriter *writer, Error &error);
    bool savePose(ByteArray &bytes, Error &error);
    bool saveArchive(const String &prefix, Archiver &archiver, Error &error);
    bool saveArchive(IFileWriter *writer, Error &error);
    void setupAllBindings();
    void upload();
    void uploadArchive(const Archiver &archiver, Progress &progress, Error &error);
    bool uploadArchive(ISeekableReader *reader, Progress &progress, Error &error);
    nanoem_u32_t createAllImages();
    void loadAllImages(Progress &progress, Error &error);
    void readLoadCommandMessage(const Nanoem__Application__Command *messagePtr);
    void writeLoadCommandMessage(Error &error);
    void writeDeleteCommandMessage(Error &error);
    void clear();
    void destroy();
    void synchronizeMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount,
        PhysicsEngine::SimulationTimingType timing);
    void synchronizeAllRigidBodiesTransformFeedbackFromSimulation(PhysicsEngine::RigidBodyFollowBoneType followType);
    void synchronizeAllRigidBodiesTransformFeedbackToSimulation();
    void performAllBonesTransform();
    void performAllMorphsDeform(bool resetAll);
    void deformAllMorphs(bool checkDirty);
    void markStagingVertexBufferDirty();
    void updateStagingVertexBuffer();
    void resetLanguage();
    void registerUpdateActiveBoneTransformCommand(const Vector3 &translation, const Quaternion &orientation);
    void registerResetBoneSetTransformCommand(
        const model::Bone::Set &boneSet, const model::BindPose &lastBindPose, ResetType type);
    void registerResetActiveBoneTransformCommand(ResetType type);
    void registerResetMorphSetWeightsCommand(const model::Morph::Set &morphSet);
    void registerResetAllMorphWeightsCommand();
    void removeBone(const nanoem_model_bone_t *value);
    void removeBone(const String &value);
    void removeMorph(const nanoem_model_morph_t *value);
    void removeMorph(const String &value);
    void pushUndo(undo_command_t *command);
    void solveAllConstraints();

    void reset() NANOEM_DECL_OVERRIDE;
    void draw(DrawType type) NANOEM_DECL_OVERRIDE;
    const IEffect *findOffscreenPassiveRenderTargetEffect(const String &ownerName) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *findOffscreenPassiveRenderTargetEffect(const String &ownerName) NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setOffscreenDefaultRenderTargetEffect(const String &ownerName) NANOEM_DECL_OVERRIDE;
    void setOffscreenPassiveRenderTargetEffect(const String &ownerName, IEffect *value) NANOEM_DECL_OVERRIDE;
    void removeOffscreenPassiveRenderTargetEffect(const String &ownerName) NANOEM_DECL_OVERRIDE;
    bool isOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setOffscreenPassiveRenderTargetEffectEnabled(const String &ownerName, bool value) NANOEM_DECL_OVERRIDE;

    void clearAllBoneBoundsRigidBodies();
    void createAllBoneBoundsRigidBodies();
    bool intersectsBoneInWindow(
        const Vector2 &devicePixelCursor, const model::Bone *bone, Vector2 &coord) const NANOEM_DECL_NOEXCEPT;
    bool intersectsBoneInViewport(
        const Vector2 &devicePixelCursor, const model::Bone *bone, Vector2 &coord) const NANOEM_DECL_NOEXCEPT;

    void saveBindPose(model::BindPose &value) const;
    void restoreBindPose(const model::BindPose &value);
    void resetAllBoneTransforms();
    void resetAllBoneLocalTransform();
    void resetAllBoneMorphTransform();
    void resetAllMaterials();
    void resetAllMorphs();
    void resetAllVertices();
    void initializeAllRigidBodiesTransformFeedback();
    void initializeAllSoftBodiesTransformFeedback();
    void setRigidBodiesVisualization(const model::RigidBody::VisualizationClause &clause);
    void setSoftBodiesVisualization();
    void rename(const String &value, nanoem_language_type_t language);

    void addAttachment(const String &name, const URI &fullPath);
    void removeAttachment(const String &name);
    FileEntityMap attachments() const;

    model::Bone::OutsideParentMap allOutsideParents() const;
    StringPair findOutsideParent(const nanoem_model_bone_t *key) const;
    bool hasOutsideParent(const nanoem_model_bone_t *key) const NANOEM_DECL_NOEXCEPT;
    void setOutsideParent(const nanoem_model_bone_t *key, const StringPair &value);
    void removeOutsideParent(const nanoem_model_bone_t *key);
    sg_image *uploadImage(const String &filename, const sg_image_desc &desc) NANOEM_DECL_OVERRIDE;
    bool isBoneSelectable(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT;
    bool isMaterialSelected(const nanoem_model_material_t *material) const NANOEM_DECL_NOEXCEPT;
    bool isBoneConnectionDrawable(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT;
    void drawBoneConnections(
        const nanoem_model_bone_t *bone, const nanoem_model_bone_t *parentBone, nanoem_f32_t thickness);
    void drawBoneConnections(const Vector2 &devicePixelCursor);
    void drawBoneTooltip(const nanoem_model_bone_t *bonePtr);
    void drawConstraintConnections(const Vector2 &devicePixelCursor, const nanoem_model_constraint_t *constraint);
    void drawConstraintConnections(const Vector2 &devicePixelCursor);
    void drawConstraintsHeatMap(const nanoem_model_constraint_t *constraint);
    void drawConstraintsHeatMap();

    const Project *project() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    Project *project() NANOEM_DECL_NOEXCEPT;
    const ICamera *localCamera() const NANOEM_DECL_NOEXCEPT;
    ICamera *localCamera() NANOEM_DECL_NOEXCEPT;
    const IModelObjectSelection *selection() const NANOEM_DECL_NOEXCEPT;
    IModelObjectSelection *selection() NANOEM_DECL_NOEXCEPT;
    nanoem_u16_t handle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String name() const NANOEM_DECL_OVERRIDE;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    String canonicalName() const NANOEM_DECL_OVERRIDE;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setName(const String &value);
    String comment() const;
    void setComment(const String &value);
    void setComment(const String &value, nanoem_language_type_t language);
    nanoem_codec_type_t codecType() const NANOEM_DECL_NOEXCEPT;
    void setCodecType(nanoem_codec_type_t value);
    StringMap annotations() const;
    void setAnnotations(const StringMap &value);
    const nanoem_model_t *data() const NANOEM_DECL_NOEXCEPT;
    nanoem_model_t *data() NANOEM_DECL_NOEXCEPT;
    model::Bone *sharedFallbackBone();
    const nanoem_model_bone_t *activeBone() const NANOEM_DECL_NOEXCEPT;
    nanoem_model_bone_t *activeBone() NANOEM_DECL_NOEXCEPT;
    void setActiveBone(const nanoem_model_bone_t *value);
    const nanoem_model_constraint_t *activeConstraint() const NANOEM_DECL_NOEXCEPT;
    nanoem_model_constraint_t *activeConstraint() NANOEM_DECL_NOEXCEPT;
    void setActiveConstraint(const nanoem_model_constraint_t *value);
    const nanoem_model_morph_t *activeMorph(nanoem_model_morph_category_t category) const NANOEM_DECL_NOEXCEPT;
    nanoem_model_morph_t *activeMorph(nanoem_model_morph_category_t category) NANOEM_DECL_NOEXCEPT;
    void setActiveMorph(nanoem_model_morph_category_t category, const nanoem_model_morph_t *value);
    const nanoem_model_morph_t *activeMorph() const NANOEM_DECL_NOEXCEPT;
    nanoem_model_morph_t *activeMorph() NANOEM_DECL_NOEXCEPT;
    void setActiveMorph(const nanoem_model_morph_t *value);
    const nanoem_model_material_t *activeMaterial() const NANOEM_DECL_NOEXCEPT;
    void setActiveMaterial(const nanoem_model_material_t *value);
    const nanoem_model_bone_t *activeOutsideParentSubjectBone() const NANOEM_DECL_NOEXCEPT;
    void setActiveOutsideParentSubjectBone(const nanoem_model_bone_t *value);
    const undo_stack_t *undoStack() const NANOEM_DECL_NOEXCEPT;
    undo_stack_t *undoStack() NANOEM_DECL_NOEXCEPT;
    Matrix4x4 worldTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 worldTransform(const Matrix4x4 &initial) const NANOEM_DECL_NOEXCEPT;
    bool containsBone(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT;
    bool containsBone(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT;
    bool containsMorph(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT;
    bool containsMorph(const nanoem_model_morph_t *value) const NANOEM_DECL_NOEXCEPT;
    bool hasAnyDirtyBone() const NANOEM_DECL_NOEXCEPT;
    bool hasAnyDirtyMorph() const NANOEM_DECL_NOEXCEPT;
    bool isRigidBodyBound(const nanoem_model_bone_t *value) const NANOEM_DECL_NOEXCEPT;
    model::Bone::Set findInherentBoneSet(const nanoem_model_bone_t *bone) const;
    const nanoem_model_bone_t *intersectsBone(
        const Vector2 &devicePixelCursor, nanoem_rsize_t &candidateBoneIndex) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_bone_t *findBone(const nanoem_unicode_string_t *name) const;
    const nanoem_model_bone_t *findBone(const String &name) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_bone_t *findRedoBone(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_morph_t *findMorph(const nanoem_unicode_string_t *name) const;
    const nanoem_model_morph_t *findMorph(const String &name) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_morph_t *findRedoMorph(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_constraint_t *findConstraint(const nanoem_unicode_string_t *name) const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_constraint_t *findConstraint(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT;
    bool isConstraintJointBone(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT;
    bool isConstraintJointBoneActive(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT;
    bool isConstraintEffectorBone(const nanoem_model_bone_t *bone) const NANOEM_DECL_NOEXCEPT;
    model::Bone::ListTree parentBoneTree() const;
    void getAllImageViews(ImageViewMap &value) const NANOEM_DECL_OVERRIDE;
    URI resolveImageURI(const String &filename) const;
    BoundingBox boundingBox() const NANOEM_DECL_NOEXCEPT;
    String filename() const;
    const URI *fileURIPtr() const NANOEM_DECL_NOEXCEPT;
    URI fileURI() const NANOEM_DECL_OVERRIDE;
    URI resolvedFileURI() const;
    void setFileURI(const URI &value);
    const IEffect *activeEffect() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *activeEffect() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setActiveEffect(IEffect *value) NANOEM_DECL_OVERRIDE;
    const IEffect *passiveEffect() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IEffect *passiveEffect() NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setPassiveEffect(IEffect *value) NANOEM_DECL_OVERRIDE;
    UserData userData() const;
    void setUserData(const UserData &value);
    TransformCoordinateType transformCoordinateType() const NANOEM_DECL_NOEXCEPT;
    AxisType transformAxisType() const NANOEM_DECL_NOEXCEPT;
    void setTransformAxisType(AxisType value);
    void setTransformCoordinateType(TransformCoordinateType value);
    void toggleTransformCoordinateType();
    Vector4 edgeColor() const NANOEM_DECL_NOEXCEPT;
    void setEdgeColor(const Vector4 &value);
    nanoem_f32_t edgeSize() const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t edgeSizeScaleFactor() const NANOEM_DECL_NOEXCEPT;
    void setEdgeSizeScaleFactor(nanoem_f32_t value);
    nanoem_f32_t opacity() const NANOEM_DECL_NOEXCEPT;
    void setOpacity(nanoem_f32_t value);
    bool isMorphWeightFocused() const NANOEM_DECL_NOEXCEPT;
    void setMorphWeightFocused(bool value);
    bool isShowAllBones() const NANOEM_DECL_NOEXCEPT;
    void setShowAllBones(bool value);
    bool isShowAllVertexPoints() const NANOEM_DECL_NOEXCEPT;
    void setShowAllVertexPoints(bool value);
    bool isShowAllVertexFaces() const NANOEM_DECL_NOEXCEPT;
    void setShowAllVertexFaces(bool value);
    bool isShowAllRigidBodies() const NANOEM_DECL_NOEXCEPT;
    void setShowAllRigidBodies(bool value);
    bool isShowAllJoints() const NANOEM_DECL_NOEXCEPT;
    void setShowAllJoints(bool value);
    bool isGroundShadowEnabled() const NANOEM_DECL_NOEXCEPT;
    void setGroundShadowEnabled(bool value);
    bool isShadowMapEnabled() const NANOEM_DECL_NOEXCEPT;
    void setShadowMapEnabled(bool value);
    bool isPhysicsSimulationEnabled() const NANOEM_DECL_NOEXCEPT;
    void setPhysicsSimulationEnabled(bool value);
    bool isAddBlendEnabled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setAddBlendEnabled(bool value) NANOEM_DECL_OVERRIDE;
    bool isVisible() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setVisible(bool value) NANOEM_DECL_OVERRIDE;
    bool isDirty() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setDirty(bool value);
    bool isTranslucent() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isUploaded() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    tinystl::unordered_map<const nanoem_model_label_t *, bool, TinySTLAllocator> checkedState;

private:
    struct LoadingImageItem;
    typedef tinystl::vector<LoadingImageItem *, TinySTLAllocator> LoadingImageItemList;
    typedef tinystl::unordered_map<String, Image *, TinySTLAllocator> ImageMap;
    typedef tinystl::unordered_map<String, const nanoem_model_bone_t *, TinySTLAllocator> BoneHashMap;
    typedef tinystl::unordered_map<String, const nanoem_model_morph_t *, TinySTLAllocator> MorphHashMap;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, const nanoem_model_constraint_t *, TinySTLAllocator>
        ConstraintMap;
    typedef tinystl::unordered_map<nanoem_u32_t, Vector2UI16, TinySTLAllocator> ImageSizeMap;
    typedef tinystl::pair<int, model::Vertex::List> BoneVertexPair;
    struct ParallelSkinningTaskData {
        ParallelSkinningTaskData(Model *model, const IDrawable::DrawType type, nanoem_f32_t edgeSizeFactor);
        ~ParallelSkinningTaskData() NANOEM_DECL_NOEXCEPT;
        const Model *m_model;
        const IDrawable::DrawType m_drawType;
        const nanoem_f32_t m_edgeSizeScaleFactor;
        model::Material::BoneIndexHashMap *m_boneIndices;
        nanoem_u8_t *m_output;
        nanoem_model_material_t *const *m_materials;
        nanoem_model_vertex_t *const *m_vertices;
        nanoem_rsize_t m_numVertices;
    };
    struct DrawArrayBuffer {
        DrawArrayBuffer();
        ~DrawArrayBuffer() NANOEM_DECL_NOEXCEPT;
        void destroy();
        sg_buffer m_buffer;
        tinystl::vector<sg::LineVertexUnit, TinySTLAllocator> m_vertices;
    };
    struct DrawIndexedBuffer {
        typedef nanoem_u32_t IndexType;
        DrawIndexedBuffer();
        ~DrawIndexedBuffer() NANOEM_DECL_NOEXCEPT;
        nanoem_u32_t fillShape(const par_shapes_mesh *mesh, const nanoem::Vector4 &color);
        void destroy();
        sg_buffer m_vertexBuffer;
        sg_buffer m_indexBuffer;
        tinystl::vector<sg::LineVertexUnit, TinySTLAllocator> m_vertices;
        tinystl::vector<IndexType, TinySTLAllocator> m_indices;
        Vector4 m_color;
    };
    struct OffscreenPassiveRenderTargetEffect {
        IEffect *m_passiveEffect;
        bool m_enabled;
    };
    typedef tinystl::unordered_map<const par_shapes_mesh_s *, DrawIndexedBuffer, TinySTLAllocator> RigidBodyBuffers;
    typedef tinystl::unordered_map<const par_shapes_mesh_s *, DrawIndexedBuffer, TinySTLAllocator> JointBuffers;
    typedef tinystl::unordered_map<String, OffscreenPassiveRenderTargetEffect, TinySTLAllocator>
        OffscreenPassiveRenderTargetEffectMap;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, const nanoem_model_rigid_body_t *, TinySTLAllocator>
        BoneBoundRigidBodyMap;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, const nanoem_model_constraint_t *, TinySTLAllocator>
        ResolveConstraintJointParentMap;
    typedef void (*DispatchParallelTasksIterator)(void *, size_t);

    static int compareBoneVertexList(const void *a, const void *b);
    static void handlePerformSkinningVertexTransform(void *opaque, size_t index);
    static void setCommonPipelineDescription(sg_pipeline_desc &desc);

    const IEffect *activeEffect(const model::Material *material) const NANOEM_DECL_NOEXCEPT;
    IEffect *activeEffect(model::Material *material);
    IEffect *internalEffect(model::Material *material);
    const Image *createImage(const nanoem_unicode_string_t *path, sg_wrap wrap, nanoem_u32_t flags);
    internal::LineDrawer *lineDrawer();
    void splitBonesPerMaterial(model::Material::BoneIndexHashMap &boneIndexHash) const;
    void bindConstraint(nanoem_model_constraint_t *constraintPtr);
    void applyAllBonesTransform(PhysicsEngine::SimulationTimingType timing);
    void internalClear();
    void internalSetOutsideParent(const nanoem_model_bone_t *key, const StringPair &value);
    void createVertexIndexBuffers();
    void initializeVertexBuffers();
    void initializeVertexBufferByteArray();
    void internalUpdateStagingVertexBuffer(nanoem_u8_t *ptr, nanoem_rsize_t numVertices);
    void clearAllLoadingImageItems();
    void predeformMorph(const nanoem_model_morph_t *morphPtr);
    void deformMorph(const nanoem_model_morph_t *morphPtr, bool checkDirty);
    void solveConstraint(
        const nanoem_model_constraint_t *constraintPtr, int numIterations, nanoem_unicode_string_factory_t *factory);
    void synchronizeBoneMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount,
        PhysicsEngine::SimulationTimingType timing);
    void synchronizeMorphMotion(const Motion *motion, nanoem_frame_index_t frameIndex, nanoem_f32_t amount);
    void synchronizeAllConstraintStates(const nanoem_motion_model_keyframe_t *keyframe);
    void synchronizeAllOutsideParents(const nanoem_motion_model_keyframe_t *keyframe);
    void synchronizeAllRigidBodyKinematics(const Motion *motion, nanoem_frame_index_t frameIndex);
    void dispatchParallelTasks(DispatchParallelTasksIterator iterator, void *opaque, size_t iterations);
    bool saveAllAttachments(
        const String &prefix, const FileEntityMap &allAttachments, Archiver &archiver, Error &error);
    bool getVertexIndexBuffer(const model::Material *material, IPass::Buffer &buffer) const NANOEM_DECL_NOEXCEPT;
    bool getEdgeIndexBuffer(const model::Material *material, IPass::Buffer &buffer) const NANOEM_DECL_NOEXCEPT;
    Vector4 connectionBoneColor(
        const nanoem_model_bone_t *bone, const Vector4 &base, bool enableFixedAxis) const NANOEM_DECL_NOEXCEPT;
    Vector4 hoveredBoneColor(const Vector4 &inactive, bool selected) const NANOEM_DECL_NOEXCEPT;
    void drawColor(bool scriptExternalColor);
    void drawEdge(nanoem_f32_t edgeSizeScaleFactor);
    void drawGroundShadow();
    void drawShadowMap();
    void drawAllJointShapes();
    void drawAllRigidBodyShapes();
    void drawAllVertexPoints();
    void drawAllVertexFaces();
    void drawJointShape(const nanoem_model_joint_t *jointPtr);
    void drawRigidBodyShape(const nanoem_model_rigid_body_t *bodyPtr);
    void drawBoneConnection(const nanoem_model_bone_t *from, const Vector3 &destinationPosition, const Vector4 &color,
        nanoem_f32_t circleRadius, nanoem_f32_t thickness);
    void drawBonePoint(const Vector2 &devicePixelCursor, const nanoem_model_bone_t *bonePtr, const Vector4 &inactive,
        const Vector4 &hovered);
    void drawConstraintPoint(const Vector4 &position, int j, int numIterations);

    const nanoem_u16_t m_handle;
    Project *m_project;
    ICamera *m_camera;
    IModelObjectSelection *m_selection;
    internal::LineDrawer *m_drawer;
    model::ISkinDeformer *m_skinDeformer;
    OffscreenPassiveRenderTargetEffectMap m_offscreenPassiveRenderTargetEffects;
    DrawArrayBuffer m_drawAllPoints;
    DrawIndexedBuffer m_drawAllLines;
    RigidBodyBuffers m_drawRigidBody;
    JointBuffers m_drawJoint;
    nanoem_model_t *m_opaque;
    undo_stack_t *m_undoStack;
    const nanoem_model_morph_t *m_activeMorphPtr[NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM];
    const nanoem_model_constraint_t *m_activeConstraintPtr;
    const nanoem_model_material_t *m_activeMaterialPtr;
    ByteArray m_vertexBufferData;
    tinystl::pair<const nanoem_model_bone_t *, const nanoem_model_bone_t *> m_activeBonePairPtr;
    tinystl::pair<IEffect *, IEffect *> m_activeEffectPtrPair;
    Image *m_screenImage;
    LoadingImageItemList m_loadingImageItems;
    ImageMap m_imageHandles;
    model::Material::BoneIndexHashMap m_boneIndexHashes;
    BoneHashMap m_bones;
    MorphHashMap m_morphs;
    ConstraintMap m_constraints;
    StringList m_redoBoneNames;
    StringList m_redoMorphNames;
    model::Bone::OutsideParentMap m_outsideParents;
    FileEntityMap m_imageURIs;
    FileEntityMap m_attachmentURIs;
    BoneBoundRigidBodyMap m_boneBoundRigidBodies;
    ResolveConstraintJointParentMap m_constraintJointBones;
    model::Bone::SetTree m_inherentBones;
    model::Bone::Set m_constraintEffectorBones;
    model::Bone::ListTree m_parentBoneTree;
    model::Bone *m_sharedFallbackBone;
    BoundingBox m_boundingBox;
    UserData m_userData;
    StringMap m_annotations;
    sg_buffer m_vertexBuffers[2];
    sg_buffer m_indexBuffer;
    Vector4 m_edgeColor;
    AxisType m_transformAxisType;
    TransformCoordinateType m_transformCoordinateType;
    URI m_fileURI;
    String m_name;
    String m_comment;
    String m_canonicalName;
    nanoem_u32_t m_states;
    nanoem_f32_t m_edgeSizeScaleFactor;
    nanoem_f32_t m_opacity;
    void *m_dispatchParallelTaskQueue;
    mutable int m_countVertexSkinningNeeded;
    int m_stageVertexBufferIndex;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_H_ */