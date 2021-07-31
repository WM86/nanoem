/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_EFFECT_H_
#define EMAPP_PLUGIN_SDK_EFFECT_H_

#include "Common.h"

/**
 * \defgroup emapp
 * @{
 */

/**
 * \defgroup emapp_plugin_effect nanoem Effect Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION                                                          \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MAJOR,            \
        NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MINOR)

NANOEM_DECL_ENUM(int, nanoem_application_plugin_effect_option_t) { NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_UNKNOWN = -1,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION = NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION, NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_ESSL, NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_HLSL,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_MSL, NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_SPIRV,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_ENABLE_MME_MIPMAP, NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_MAX_ENUM };

/**
 * \brief The opaque effect compiler plugin object
 */
typedef struct nanoem_application_plugin_effect_compiler_t nanoem_application_plugin_effect_compiler_t;

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginEffectCompilerGetABIVersion(void);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerInitialize(void);

/**
 * \brief Create an opaque effect compiler plugin object
 *
 * \return NANOEM_DECL_API nanoem_application_plugin_effect_compiler_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_effect_compiler_t *APIENTRY nanoemApplicationPluginEffectCompilerCreate();

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerGetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param key
 * \param value
 * \param size
 * \param status
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerSetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, const void *value, nanoem_u32_t size,
    nanoem_i32_t *status);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param size
 * \return NANOEM_DECL_API const char* const* APIENTRY
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginEffectCompilerGetAvailableExtensions(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t *size);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param path
 * \param size
 * \return NANOEM_DECL_API nanoem_u8_t* APIENTRY
 */
NANOEM_DECL_API nanoem_u8_t *APIENTRY nanoemApplicationPluginEffectCompilerCreateBinaryFromFile(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, nanoem_u32_t *size);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param source
 * \param length
 * \param size
 * \return NANOEM_DECL_API nanoem_u8_t* APIENTRY
 */
NANOEM_DECL_API nanoem_u8_t *APIENTRY nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *source, nanoem_u32_t length, nanoem_u32_t *size);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param path
 * \param data
 * \param size
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerAddIncludeSource(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, const nanoem_u8_t *data, nanoem_u32_t size);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEffectCompilerGetFailureReason(
    const nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEffectCompilerGetRecoverySuggestion(
    const nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief
 *
 * \param plugin The opaque effect compiler plugin object
 * \param data
 * \param size
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerDestroyBinary(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u8_t *data, nanoem_u32_t size);

/**
 * \brief Destroy an opaque effect compiler plugin object
 *
 * \param plugin The opaque effect compiler plugin object
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerDestroy(
    nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerTerminate(void);

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_EFFECT_H_ */
