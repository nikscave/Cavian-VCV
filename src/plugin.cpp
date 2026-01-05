#include "plugin.hpp"

// REQUIRED: define the global plugin instance
Plugin* pluginInstance = nullptr;

// REQUIRED: Rack entry point
void init(Plugin* p) {
    pluginInstance = p;

    // Register your module
    p->addModel(modelCavianSequencer);
}
