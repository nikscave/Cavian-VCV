# Linux Build Fixes for Cavian-VCV

## Problem

The Linux build (.vcvplugin) was not loading in VCV Rack on Linux systems, while Windows and Mac builds worked fine.

## Root Causes

1. **Missing system libraries** - The Linux build was missing several required dependencies
2. **Insufficient linking** - The Makefile didn't link SSL/TLS libraries needed for HTTP functionality
3. **No dependency checking** - No way to verify the plugin's library dependencies

## Changes Made

### 1. Updated GitHub Actions (`.github/workflows/build.yml`)

Added missing Linux dependencies:
- `libudev-dev` - Device handling
- `libfontconfig1-dev` - Font rendering
- `libssl-dev` - SSL/TLS support for HTTP client

### 2. Updated Makefile

Added Linux-specific linking flags:
```makefile
ifdef ARCH_LIN
    LDFLAGS += -lpthread -lssl -lcrypto
endif
```

### 3. Added Test Workflow (`.github/workflows/test-linux-plugin.yml`)

New workflow to verify dependencies by:
- Building the plugin
- Extracting the .vcvplugin archive
- Running `ldd plugin.so` to check all library dependencies
- Uploading results for debugging

## How to Use These Fixes

### Option 1: Merge the branch
1. Review the changes in `fix/linux-dependencies` branch
2. Merge into main branch
3. Push to trigger new GitHub Actions build
4. Test the new Linux build

### Option 2: Manual application
If you prefer to apply changes manually:

```bash
# Update apt dependencies
sudo apt-get install -y \
  libgl-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev \
  libasound2-dev \
  libudev-dev \
  libfontconfig1-dev \
  libssl-dev

# Update Makefile - add these lines after ARCH_WIN block
ifdef ARCH_LIN
    LDFLAGS += -lpthread -lssl -lcrypto
endif
```

## Testing the Fix

1. **Run the test workflow:**
   - Go to Actions tab
   - Select "Test Linux Plugin Dependencies"
   - Click "Run workflow"

2. **Check the output:**
   - Look for "not found" in ldd output
   - All dependencies should show a path (e.g., `/usr/lib/x86_64-linux-gnu/libssl.so`)

3. **Test in VCV Rack:**
   - Download the new .vcvplugin from releases
   - Install and check if module appears

## Expected Build Size Increase

After these fixes, the Linux plugin size should increase from ~93KB to closer to Windows size (~140KB), as all necessary libraries are now properly linked.

## Additional Troubleshooting

If the plugin still doesn't load:

1. **Check VCV Rack logs:**
   ```bash
   cat ~/.vcvrack/logs/Rack.log
   ```

2. **Check plugin dependencies manually:**
   ```bash
   # Extract the .vcvplugin (it's a zip file)
   unzip CavianSequencer-*.vcvplugin -d plugin_extract
   cd plugin_extract
   ldd plugin.so
   ```

3. **Common issues:**
   - Missing `libssl.so.1.1` (older systems)
   - Missing `libcurl.so.4` (if using HTTP)
   - Architecture mismatch (check plugin.so is x86_64)

## References

- [VCV Rack Plugin Development](https://vcvrack.com/manual/PluginDevelopment)
- [Rack SDK GitHub](https://github.com/VCVRack/Rack)
