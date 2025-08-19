#include "vfs.h"

#include "assets/kasset_types.h"
#include "containers/darray.h"
#include "kdebug/kassert.h"
#include "logger.h"
#include "memory/kmemory.h"
#include "platform/filesystem.h"
#include "platform/kpackage.h"
#include "strings/kstring.h"

static b8 process_manifest_refs(vfs_state* state, const asset_manifest* manifest);

b8 vfs_initialize(u64* memory_requirement, vfs_state* state, const vfs_config* config) {
    if (!memory_requirement) {
        KERROR("vfs_initialize requires a valid pointer to memory_requirement.");
        return false;
    }

    *memory_requirement = sizeof(vfs_state);
    if (!state) {
        return true;
    }
    else if (!config) {
        KERROR("vfs_initialize requires a valid pointer to config.");
        return false;
    }

    state->packages = darray_create(kpackage);

    // TODO: For release builds, look at binary file.
    //FIXME:hardcord rubbish.
    const char* file_path = "../testbed.kapp/asset_manifest.kson";
    asset_manifest manifest = { 0 };
    if (!kpackage_parse_manifest_file_content(file_path, &manifest)) {
        KERROR("Failed to parse primary asset manifest. See logs for details.");
        return false;
    }

    kpackage primary_package = { 0 };
    if (!kpackage_create_from_manifest(&manifest, &primary_package)) {
        KERROR("Failed to create package from primary asset manifest. See logs for details.");
        return false;
    }
    darray_push(state->packages, primary_package);

    // Examine primary package references and load them as needed.
    if (!process_manifest_refs(state, &manifest)) {
        KERROR("Failed to process manifest reference. See logs for details.");
        kpackage_manifest_destroy(&manifest);
        return false;
    }

    kpackage_manifest_destroy(&manifest);

    return true;
}

void vfs_shutdown(vfs_state* state) {
    if (state) {
        if (state->packages) {
            u32 package_count = darray_length(state->packages);
            for (u32 i = 0;i < package_count;++i) {
                kpackage_destroy(&state->packages[i]);
            }
            darray_destroy(state->packages);
            state->packages = 0;
        }
    }
}

void vfs_request_asset(vfs_state* state, struct kasset_metadata* meta, b8 is_binary, b8 get_source, u32 context_size, const void* context, PFN_on_asset_loaded_callback callback) {

    if (!state || !meta || !callback) {
        KERROR("vfs_request_asset requires state, meta and callback to be provided.");
    }



}

void vfs_request_asset_sync(vfs_state* state, struct kasset_metadata* meta, b8 is_binary, b8 get_source, u32 context_size, const void* context, vfs_asset_data* out_data) {
    if (!out_data) {
        KERROR("vfs_request_asset_sync requires a valid pointer to out_data. Nothing can or will be done.");
        return;
    }

    if (!state || !meta) {
        KERROR("vfs_request_asset_sync requires state, name and callback to be provided.");
        out_data->result = VFS_REQUEST_RESULT_INTERNAL_FAILURE;
        return;
    }

    kzero_memory(out_data, sizeof(vfs_asset_data));

    //Take a copy of the context if provided. This will need to be freed by the caller.
    if (context_size) {
        KASSERT_MSG(context, "Called vfs_request_asset with a context_size, but not a context. Check yourself before you wreck yourself.");
        out_data->context_size = context_size;
        out_data->context = kallocate(context_size, MEMORY_TAG_PLATFORM);
        kcopy_memory(out_data->context, context, out_data->context_size);
    }
    else {
        out_data->context_size = 0;
        out_data->context = 0;
    }

    kasset_name* name = &meta->name;
    KDEBUG("Loading asset '%s' of type '%s' from package '%s'...", name->asset_name, name->asset_type, name->package_name);

    u32 package_count = darray_length(state->packages);
    for (u32 i = 0;i < package_count;++i) {
        kpackage* package = &state->packages[i];
        // TODO: Optimization: Take a hash of the package names, then a hash of the requested package name
        // and compare those instead.
        if (strings_equali(package->name, name->package_name)) {

            kzero_memory(out_data, sizeof(vfs_asset_data));
            // Determine if the asset type is text.
            kpackage_result
            vfs_asset_data data = { 0 };
            if (is_binary) {
                data.bytes = kpackage_asset_bytes_get(package, name->asset_name, &data.size);
                if (!data.bytes) {
                    KERROR("Failed to load binary asset. See logs for details.");
                    data.success = false;
                    data.flags |= VFS_ASSET_FLAG_BINARY_BIT;
                    return;
                }
            }
            else {
                data.text = kpackage_asset_text_get(package, name->asset_name, &data.size);
                if (!data.text) {
                    KERROR("Failed to text load asset. See logs for details.");
                    data.success = false;
                    return;
                }
            }

            // Take a copy of the context if provided. This will be freed immediately after the callback is made below.
           // This means the context should be immediately consumed by the callback before any async
           // work is done. 
            if (context_size) {
                KASSERT_MSG(context, "Called vfs_request_asset with a context_size, but not a context. Check yourself before you wreck yourself.");
                data.context_size = context_size;
                data.context = kallocate(context_size, MEMORY_TAG_PLATFORM);
                kcopy_memory(data.context, context, data.context_size);
            }
            else {
                data.context_size = 0;
                data.context = 0;
            }
            data.success = true;
            callback(name->asset_name, data);

            //Cleanup the context.
            if (data.context_size) {
                kfree(data.context, data.context_size, MEMORY_TAG_PLATFORM);
                data.context = 0;
                data.context_size = 0;
            }

            return;
        }
    }
}

b8 vfs_asset_write(vfs_state* state, const kasset* asset, b8 is_binary, u64 size, void* data) {
    u32 package_count = darray_length(state->packages);
    for (u32 i = 0;i < package_count;++i) {
        kpackage* package = &state->packages[i];
        // TODO: Optimization: Take a hash of the package names, then a hash of the requested package name
        // and compare those instead.
        if (strings_equali(package->name, asset->meta.name.package_name)) {
            if (is_binary) {
                return kpackage_asset_bytes_write(package, asset->meta.name.fully_qualified_name, size, data);
            }
            else {
                return kpackage_asset_text_write(package, asset->meta.name.fully_qualified_name, size, data);
            }
        }
    }

    KERROR("vfs_asset_write:Unable to find package named '%s'.", asset->meta.name.package_name);
}

static b8 process_manifest_refs(vfs_state* state, const asset_manifest* manifest) {
    b8 success = true;
    if (manifest->references) {
        u32 ref_count = darray_length(manifest->references);
        for (u32 i = 0;i < ref_count;++i) {
            asset_manifest_reference* ref = &manifest->references[i];

            //Don't load the same package more than once.
            b8 exists = false;
            u32 package_count = darray_length(state->packages);
            for (u32 j = 0;j < package_count;++j) {
                if (strings_equali(state->packages[j].name, ref->name)) {
                    KTRACE("Package '%s' already loaded, skipping.", ref->name);
                    exists = true;
                    break;
                }
            }
            if (exists) {
                continue;
            }

            asset_manifest new_manifest = { 0 };
            if (!kpackage_parse_manifest_file_content(ref->path, &new_manifest)) {
                KERROR("Failed to parse asset manifest. See logs for details.");
                return false;
            }

            kpackage package = { 0 };
            if (!kpackage_create_from_manifest(&new_manifest, &package)) {
                KERROR("Failed to create package from asset manifest. See logs for details.");
                return false;
            }

            darray_push(state->packages, package);

            //Process reference
            if (!process_manifest_refs(state, &new_manifest)) {
                KERROR("Failed to process manifest reference. See logs for details.");
                kpackage_manifest_destroy(&new_manifest);
                success = false;
                break;
            }
            else {
                kpackage_manifest_destroy(&new_manifest);
            }
        }
    }

    return success;
}

