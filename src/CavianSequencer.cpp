#include "plugin.hpp"
#include <cstring>
#include <jansson.h>  
#include "HTTPClient.hpp"



// ===================================================================
// COLOR PALETTE
// ===================================================================

// Primary States
static const NVGcolor CLR_MUTED = nvgRGBA(239, 68, 68, 255);        // Red but muted
static const NVGcolor CLR_INACTIVE = nvgRGBA(25, 25, 35, 255);      // Dark background
static const NVGcolor CLR_GREEN = nvgRGBA(16, 185, 129, 255);     // Green
static const NVGcolor CLR_GATE = nvgRGBA(255, 50, 50, 255);        // Red ; 

// Classic Eurorack Colors
static const NVGcolor CLR_YELLOW = nvgRGBA(251, 250, 36, 255);      // Warm yellow
static const NVGcolor CLR_ORANGE = nvgRGBA(249, 115, 22, 255);      // Bright orange
static const NVGcolor CLR_CYAN = nvgRGBA(34, 211, 238, 255);        // Electric cyan
static const NVGcolor CLR_MAGENTA = nvgRGBA(236, 72, 153, 255);     // Hot pink/magenta
static const NVGcolor CLR_PURPLE = nvgRGBA(168, 85, 247, 255);      // Deep purple
static const NVGcolor CLR_LIME = nvgRGBA(132, 204, 22, 255);        // Lime green
static const NVGcolor CLR_BLUE = nvgRGBA(59, 130, 246, 255);        // Bright blue
static const NVGcolor CLR_TEAL = nvgRGBA(20, 184, 166, 255);        // Teal

// Neutrals & Accents
static const NVGcolor CLR_WHITE = nvgRGBA(255, 255, 255, 255);      // Pure white
static const NVGcolor CLR_GRAY_LIGHT = nvgRGBA(156, 163, 175, 255); // Light gray
static const NVGcolor CLR_GRAY = nvgRGBA(107, 114, 128, 255);       // Medium gray
static const NVGcolor CLR_GRAY_DARK = nvgRGBA(55, 65, 81, 255);     // Dark gray
static const NVGcolor CLR_BLACK = nvgRGBA(0, 0, 0, 255);            // Pure black

// Transparency Variants (for glows, overlays)
static const NVGcolor CLR_GLOW_WHITE = nvgRGBA(255, 255, 255, 40);  // Subtle white glow
static const NVGcolor CLR_STROKE_WHITE = nvgRGBA(255, 255, 255, 20); // Subtle white stroke
static const NVGcolor CLR_GLOW_SOFT = nvgRGBA(255, 255, 255, 60);   // Brighter glow
static const NVGcolor CLR_SHADOW = nvgRGBA(0, 0, 0, 80);            // Soft shadow

// Classic LED/Display Colors (for that retro hardware feel)
static const NVGcolor CLR_LED_RED = nvgRGBA(255, 0, 0, 255);        // Pure LED red
static const NVGcolor CLR_LED_GREEN = nvgRGBA(0, 255, 0, 255);      // Pure LED green
static const NVGcolor CLR_LED_AMBER = nvgRGBA(255, 191, 0, 255);    // Amber/orange LED
static const NVGcolor CLR_LED_BLUE = nvgRGBA(0, 100, 255, 255);     // Classic LED blue

// Vintage Synth Colors
static const NVGcolor CLR_VINTAGE_ORANGE = nvgRGBA(255, 140, 0, 255);
static const NVGcolor CLR_VINTAGE_YELLOW = nvgRGBA(255, 215, 0, 255);
static const NVGcolor CLR_VINTAGE_CREAM = nvgRGBA(245, 222, 179, 255);
static const NVGcolor CLR_VINTAGE_BROWN = nvgRGBA(139, 90, 43, 255);
static const NVGcolor CLR_VINTAGE_TEAL = nvgRGBA(0, 128, 128, 255);


// === FORWARD DECLARATIONS ===
struct CavianSequencer;







//custom jack output
struct CavianPort : app::SvgPort {
    CavianPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/GATES.svg")));
    }
};

//custom clock OUT
struct ClockOutPort : app::SvgPort {
    ClockOutPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/CLOCKOUT.svg")));
    }
};

//custom clock IN
struct ClockInPort : app::SvgPort {
    ClockInPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/CLOCKIN.svg")));
    }
};

//custom reset OUT
struct ResetOutPort : app::SvgPort {
    ResetOutPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/RESETOUT.svg")));
    }
};

//custom reset IN
struct ResetInPort : app::SvgPort {
    ResetInPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/RESETIN.svg")));
    }
};

// ============================================================================
// BASE CLICKABLE CIRCLE DISPLAY
// ============================================================================
struct ClickableCircleDisplay : Widget {
    CavianSequencer* module;
    bool pressed = false;
    
    ClickableCircleDisplay() {
        box.size = mm2px(Vec(10, 10));
    }
    

    virtual std::string getText() = 0;
    virtual NVGcolor getColor() = 0;
    virtual NVGcolor getTextColor() {
        return nvgRGBA(255, 255, 255, 255);
    }
	    virtual bool isButtonActive() {
        return false;  // Default: not active
    }
    virtual NVGcolor getGlowColor() {
        // Default: white glow - override for custom colors
        return nvgRGBA(255, 255, 255, 180);
    }
        virtual NVGcolor getBevelColor() {
        // Default: white bevel for most buttons
        return nvgRGBA(255, 255, 255, 80);
    }
	
    void onButton(const event::Button& e) override {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
            e.consume(this);
        } else if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = false;
        }
    }
    
    void draw(const DrawArgs& args) override;
};



// === CUSTOM BUTTON WIDGET ===
struct CavianButton : app::ParamWidget {
    CavianSequencer* module;
    int buttonIndex;

    // ============================================================================
    // BUTTON STATE HELPER STRUCT
    // ============================================================================
    struct ButtonState {
        uint8_t value = 0;
        bool isCurrentStep = false;
        bool isLoopControl = false;
        bool isMuteButton = false;
        bool isCopyPasteButton = false;
        bool shouldPulseLoop = false;
        bool shouldHighlightLoopSelect = false;
        const char* label = nullptr;
		bool shouldPulseRed = false;
    };



    CavianButton() {
        box.size = mm2px(Vec(11, 11));
    }

    // === METHOD DECLARATIONS ===
    void draw(const DrawArgs& args) override;

    CavianButton::ButtonState getButtonState();
    void calculateVerticalModeState(CavianSequencer* m, int row, int col, CavianButton::ButtonState& bs);
    NVGcolor getButtonColor(const CavianButton::ButtonState& bs);

    void drawButtonBody(const DrawArgs& args, const CavianButton::ButtonState& bs, NVGcolor bgColor);
    void drawIndicators(const DrawArgs& args, const CavianButton::ButtonState& bs);
    void drawLabel(const DrawArgs& args, const char* label);
    void drawHoverEffect(const DrawArgs& args);

    void onButton(const event::Button& e) override;

	void onDragHover(const event::DragHover& e) override;



    void onHover(const HoverEvent& e) override {
        e.consume(this);
        ParamWidget::onHover(e);
    }

void onDragEnd(const event::DragEnd& e) override;


    void onEnter(const event::Enter& e) override;
    void onLeave(const event::Leave& e) override;
	


};

// === MUTE BUTTON WIDGET ===
struct MuteButton : app::ParamWidget {
    CavianSequencer* module;
    int channelIndex;
    
    MuteButton() {
        box.size = mm2px(Vec(6, 6));
    }
    
    void draw(const DrawArgs& args) override;
    void onButton(const ButtonEvent& e) override;
};


// === NAVIGATION DISPLAY ===
struct NavDisplay : Widget {
    CavianSequencer* module;
    std::string label;
    int* valuePtr;
    Vec labelOffset = Vec(0, 0);
	// size of the group preset and channel labels in top secion
    NavDisplay() {
        box.size = mm2px(Vec(15, 4));
    }
    
    void draw(const DrawArgs& args) override;  
};






struct ColumnLabel : Widget {
    std::string text;
    NVGcolor color;
    
    ColumnLabel(std::string t, NVGcolor c) : text(t), color(c) {
        box.size = mm2px(Vec(7, 3));
    }
    
    void draw(const DrawArgs& args) override {
        nvgFontSize(args.vg, 7);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x / 2, box.size.y / 2, text.c_str(), NULL);
    }
};



// === SWING TEMPLATE NAMES ===
const char* SWING_TEMPLATE_NAMES[] = {
    "Straight",
    "8th Swing",
    "Heavy 8th",
    "Triplet Feel",
    "16th Swing",
    "Push-Pull",
    "Accelerando",
    "Ritardando"
};




// === MAIN SEQUENCER MODULE ===
struct CavianSequencer : Module {
    enum ParamId {
        BPM_PARAM,
        RUN_PARAM,
        // Navigation
        GROUP_PREV_PARAM,
        GROUP_NEXT_PARAM,
        PRESET_PREV_PARAM,
        PRESET_NEXT_PARAM,
        CHANNEL_PREV_PARAM,
        CHANNEL_NEXT_PARAM,
        // View Mode
        VIEW_MODE_PARAM,
        // Grid (supports different view modes)
        ENUMS(STEP_BUTTONS, 64),
        // Mute
        ENUMS(MUTE_BUTTONS, 8),
        // Loop buttons
        GROUP_LOOP_PARAM,
        PRESET_LOOP_PARAM,
        SET_LOOP_PARAM,
        // Copy/Paste/Clear
        COPY_PARAM,
        PASTE_PARAM,
        CLEAR_PARAM,
		RANDOM_PARAM,
        // Swing controls
        SWING_MODE_PARAM,
        ENUMS(SWING_TEMPLATE_PARAMS, 8),
        SWING_GLOBAL_PARAM,
        ENUMS(SWING_STEP_PARAMS, 8),
        SWING_CASCADE_PRESET_PARAM,
        SWING_CASCADE_GROUP_PARAM,
        SWING_RANDOMIZE_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        CLK_INPUT,
        RESET_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        MASTER_CLK_OUTPUT,
        ENUMS(GATE_OUTPUTS, 8),
        OUTPUTS_LEN
    };
    enum LightId {
        RUN_LIGHT,
        ENUMS(VIEW_MODE_LIGHTS, 3), // Vertical, 8x8, 1x64
        GROUP_LOOP_LIGHT,
        PRESET_LOOP_LIGHT,
        SET_LOOP_LIGHT,
        SWING_MODE_LIGHT,
		    SWING_MODE_LIGHT_GREEN,
    SWING_MODE_LIGHT_AMBER,
        COPY_LIGHT,
        PASTE_LIGHT,
        CLEAR_LIGHT,
        LIGHTS_LEN
    };

    enum ViewMode {
        VERTICAL,
        HORIZONTAL_8X8,
        HORIZONTAL_64
    };

enum ConflictResolution {
    KEEP_NEWEST,        // Use timestamp
    KEEP_ESP32,         // ESP32 is master
    KEEP_VCV,           // VCV is master
    ASK_USER            // Prompt for choice
};

    // === SWING TEMPLATES (matching ESP) ===
    const int8_t swingTemplates[8][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},            // 0: Straight
        {0, 15, 0, 15, 0, 15, 0, 15},        // 1: 8th Swing
        {0, 25, 0, 25, 0, 25, 0, 25},        // 2: Heavy 8th Swing
        {0, 10, 0, 10, 0, 10, 0, 10},        // 3: Triplet Feel
        {0, 0, 15, 0, 0, 15, 0, 0},          // 4: 16th Swing
        {10, -10, 10, -10, 10, -10, 10, -10},// 5: Push-Pull
        {0, 5, 10, 15, 20, 15, 10, 5},       // 6: Accelerando
        {20, 15, 10, 5, 0, -5, -10, -15}     // 7: Ritardando
    };

		float clockPhase = 0.f;
float secondsPerStep = 0.f;

    // === DATA STRUCTURE (mirrors ESP) ===
    uint8_t caveArray[8][8][8][8]; // [group][preset][channel][step]
    int8_t swingFlat[8][8][8][8];  // Swing per step (-50 to +50)
    bool muteChannel[8];
    bool swingGlobalMode = true;
    int8_t swingGlobal[8] = {0};   // Global swing per channel
    
    // Loop settings
    bool groupLoopEnabled = false;
    bool presetLoopEnabled = false;
    bool setLoopEnabled = false;
    
	uint8_t groupLoopArray[8] = {1,1,1,1,0,0,0,0};
uint8_t presetLoopArray[8][8] = {{1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1},
                                {1,1,1,1,1,1,1,1}};
								
    // Navigation
    int activeGroup = 0;
    int activePreset = 0;
    int activeChannel = 0;
    ViewMode viewMode = VERTICAL;
    
    // Playback & Loop tracking
    int currentStep = 0;
    int loopCounter = 0;
    int lastGroupChangeLoop = -999;
    int lastPresetChangeLoop = -999;
    int previousGroup = -1;
    int previousPreset = -1;
    bool running = false;
    float phase = 0.f;
    
    // Copy/Paste system
    enum CopyType { NONE, GROUP_COPY, PRESET_COPY, CHANNEL_COPY };
    CopyType copyType = NONE;
    int copySourceIndex = -1;
    bool copyActive = false;
    bool pasteActive = false;
    bool clearArmed = false;
    bool randomArmed = false;
	
    // Copy buffers
    uint8_t groupCopyBuffer[8][8][8]; // [preset][channel][step]
    uint8_t presetCopyBuffer[8][8];   // [channel][step]
    uint8_t channelCopyBuffer[8];     // [step]
    int8_t swingCopyBuffer[8];        // For swing copy/paste
    
    // Clock & Gates
    dsp::SchmittTrigger runTrigger, clockTrigger, resetTrigger;
    dsp::SchmittTrigger groupPrevTrigger, groupNextTrigger;
    dsp::SchmittTrigger presetPrevTrigger, presetNextTrigger;
    dsp::SchmittTrigger channelPrevTrigger, channelNextTrigger;
    dsp::SchmittTrigger viewModeTrigger;
    dsp::SchmittTrigger groupLoopTrigger, presetLoopTrigger, setLoopTrigger;
    dsp::SchmittTrigger copyTrigger, pasteTrigger, clearTrigger;
    dsp::SchmittTrigger swingModeTrigger;
    dsp::SchmittTrigger swingTemplateTriggers[8];
    dsp::SchmittTrigger swingCascadePresetTrigger, swingCascadeGroupTrigger;
    dsp::SchmittTrigger swingRandomizeTrigger;
    
    float gatePulseTime[8] = {0.f};
    float swingDelay[8] = {0.f};
    bool pendingGate[8] = {false};
    float masterClockPulse = 0.f;
    
    // Preset cascading
    bool presetCascade = false;

	// check for ESP32
	bool esp32Connected = false;
	bool esp32CheckAttempted = false;
	float esp32CheckTimer = 0.f;
	const float ESP32_CHECK_INTERVAL = 2.0f; // Check every 2 seconds
	bool enableESP32Sync = false; // Toggled from right-click menu

std::string esp32PatternNames[8][8];  // ESP32's pattern names (separate from local)
bool esp32PatternsLoaded = false;


bool dragVisited[8][8];   // frame buffer
uint8_t dragState = 0;         
bool isDragging;

    // === COPY/PASTE/CLEAR VISUAL FEEDBACK ===
    int pendingCopyRow = -1;      // Row selected as source after Copy button
    int pendingPasteRow = -1;     // Row highlighted during Paste armed
    int pendingClearRow = -1;     // Row highlighted during Clear armed
    CopyType pendingCopyType = NONE;  // What type is pending paste/clear
    int pendingClearCol = -1;  // New: Track the specific column for clear highlight
	
	
// In CavianSequencer module class
float randomWeight = 0.5f; // Default 50% chance
	

	
	
FramebufferWidget* gridFramebuffer = nullptr;



    CavianSequencer() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        
        configParam(BPM_PARAM, 1.f, 400.f, 120.f, "BPM");
        configButton(RUN_PARAM, "Run");
        


        configButton(GROUP_PREV_PARAM, "▲");
        configButton(GROUP_NEXT_PARAM, "▼");
        configButton(PRESET_PREV_PARAM, "▲");
        configButton(PRESET_NEXT_PARAM, "▼");
        configButton(CHANNEL_PREV_PARAM, "▲");
        configButton(CHANNEL_NEXT_PARAM, "▼");
        
        configButton(VIEW_MODE_PARAM, "View Mode (Vertical/8x8/1x64)");
        configButton(GROUP_LOOP_PARAM, "Group Loop");
        configButton(PRESET_LOOP_PARAM, "Preset Loop");
        configButton(SET_LOOP_PARAM, "Set Loop Mode");
        
        configButton(COPY_PARAM, "Copy");
        configButton(PASTE_PARAM, "Paste");
        configButton(CLEAR_PARAM, "Clear");
   configButton(RANDOM_PARAM, "Random");      
        configButton(SWING_MODE_PARAM, "Swing Mode (Global/Per-Step)");
        for (int i = 0; i < 8; i++) {
            configButton(SWING_TEMPLATE_PARAMS + i, SWING_TEMPLATE_NAMES[i]);
        }
        configParam(SWING_GLOBAL_PARAM, -50.f, 50.f, 0.f, "Global Swing", "%");
        for (int i = 0; i < 8; i++) {
            configParam(SWING_STEP_PARAMS + i, -50.f, 50.f, 0.f, string::f("Step %d Swing", i+1), "%");
        }
        configButton(SWING_CASCADE_PRESET_PARAM, "Cascade Swing to Preset");
        configButton(SWING_CASCADE_GROUP_PARAM, "Cascade Swing to Group");
        configButton(SWING_RANDOMIZE_PARAM, "Randomize Swing");
        
        for (int i = 0; i < 64; i++) configButton(STEP_BUTTONS + i, "Step");
        for (int i = 0; i < 8; i++) configButton(MUTE_BUTTONS + i, "Mute");
        
        configInput(CLK_INPUT, "Clock");
        configInput(RESET_INPUT, "Reset");
        configOutput(MASTER_CLK_OUTPUT, "Master Clock");
        for (int i = 0; i < 8; i++) {
            configOutput(GATE_OUTPUTS + i, string::f("Channel %d", i + 1));
        }
        
        memset(caveArray, 0, sizeof(caveArray));
        memset(swingFlat, 0, sizeof(swingFlat));
        memset(muteChannel, 0, sizeof(muteChannel));
     //   memset(groupLoopArray, 0, sizeof(groupLoopArray));
     //   memset(presetLoopArray, 0, sizeof(presetLoopArray));
        memset(groupCopyBuffer, 0, sizeof(groupCopyBuffer));
        memset(presetCopyBuffer, 0, sizeof(presetCopyBuffer));
        memset(channelCopyBuffer, 0, sizeof(channelCopyBuffer));
        memset(swingCopyBuffer, 0, sizeof(swingCopyBuffer));
		
		memset(dragVisited, 0, sizeof(dragVisited));
		
		initPatternSystem();
		
    }



// for the framebuffer glox fix
bool isCurrentStep(int buttonIndex) {
    if (!running) return false;
    
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;
    
    if (viewMode == HORIZONTAL_8X8) {
        return (col == currentStep);
    } else if (viewMode == HORIZONTAL_64) {
        return (row == activePreset && col == currentStep);
    } else if (viewMode == VERTICAL) {
        return (col == 2 && row == currentStep); // Column 2 is steps in vertical mode
    }
    
    return false;
}

bool shouldPulseLoop(int buttonIndex) {
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;
    
    if (viewMode == VERTICAL) {
        // Only pulse in vertical mode on appropriate columns
        if (col == 0 && groupLoopEnabled) {
            return groupLoopArray[row];
        } else if (col == 1 && presetLoopEnabled) {
            return presetLoopArray[activeGroup][row];
        }
    }
    
    return false;
}



    void process(const ProcessArgs& args) override {
		
if (enableESP32Sync && !esp32Connected && !esp32CheckAttempted) {
    // Only check once on first enable
    esp32CheckTimer += args.sampleTime;
    
    if (esp32CheckTimer >= ESP32_CHECK_INTERVAL) {
        esp32CheckTimer = 0.f;
        checkESP32Connection();
        esp32CheckAttempted = true;
        
        if (!esp32Connected) {
                   INFO("ESP32 not found. Use the context menu to retry connection.");
        }
    }
} else if (!enableESP32Sync) {
    // Reset flags when sync is disabled
    esp32Connected = false;
    esp32CheckTimer = 0.f;
    esp32CheckAttempted = false; 
}
	
        // === RUN/STOP ===
        if (runTrigger.process(params[RUN_PARAM].getValue())) running = !running;
        lights[RUN_LIGHT].setBrightness(running ? 1.f : 0.f);

        // === VIEW MODE TOGGLE ===
        if (viewModeTrigger.process(params[VIEW_MODE_PARAM].getValue())) {
            viewMode = (ViewMode)((viewMode + 1) % 3);
        }
        lights[VIEW_MODE_LIGHTS + 0].setBrightness(viewMode == VERTICAL ? 1.f : 0.f);
        lights[VIEW_MODE_LIGHTS + 1].setBrightness(viewMode == HORIZONTAL_8X8 ? 1.f : 0.f);
        lights[VIEW_MODE_LIGHTS + 2].setBrightness(viewMode == HORIZONTAL_64 ? 1.f : 0.f);

        // === LOOP TOGGLES ===
        if (groupLoopTrigger.process(params[GROUP_LOOP_PARAM].getValue())) {
            if (!setLoopEnabled) groupLoopEnabled = !groupLoopEnabled;
        }
        if (presetLoopTrigger.process(params[PRESET_LOOP_PARAM].getValue())) {
            if (!setLoopEnabled) presetLoopEnabled = !presetLoopEnabled;
        }
        if (setLoopTrigger.process(params[SET_LOOP_PARAM].getValue())) {
            setLoopEnabled = !setLoopEnabled;
            if (!setLoopEnabled) {
                // Exiting set loop mode - clear highlighting
            }
        }
        lights[GROUP_LOOP_LIGHT].setBrightness(groupLoopEnabled ? 1.f : 0.f);
        lights[PRESET_LOOP_LIGHT].setBrightness(presetLoopEnabled ? 1.f : 0.f);
        lights[SET_LOOP_LIGHT].setBrightness(setLoopEnabled ? 1.f : 0.f);

lights[SWING_MODE_LIGHT_GREEN].setBrightness(swingGlobalMode ? 1.f : 0.f);
lights[SWING_MODE_LIGHT_AMBER].setBrightness(swingGlobalMode ? 0.f : 1.f);


        // === COPY/PASTE/CLEAR ===
        if (copyTrigger.process(params[COPY_PARAM].getValue())) {
            copyActive = true;
            pasteActive = false;
            copyType = NONE;
            copySourceIndex = -1;
        }
        if (pasteTrigger.process(params[PASTE_PARAM].getValue())) {
            if (copyType != NONE && copySourceIndex >= 0) {
                pasteActive = !pasteActive;
            }
        }
        if (clearTrigger.process(params[CLEAR_PARAM].getValue())) {
            clearArmed = !clearArmed;
        }
        lights[COPY_LIGHT].setBrightness(copyActive ? 1.f : 0.f);
        lights[PASTE_LIGHT].setBrightness(pasteActive ? 1.f : 0.f);
        lights[CLEAR_LIGHT].setBrightness(clearArmed ? 1.f : 0.f);


    static int lastGroup = -1;
    static int lastPreset = -1;
    static int lastChannel = -1;
    static bool lastRunning = false;
    static bool lastCopyActive = false;  
    static bool lastPasteActive = false;
    static bool lastClearArmed = false;
    static bool lastSetLoop = false;

    if (gridFramebuffer) {
        if (lastGroup != activeGroup ||
            lastPreset != activePreset ||
            lastChannel != activeChannel ||
            lastRunning != running ||
            lastCopyActive != copyActive ||  
            lastPasteActive != pasteActive ||
            lastClearArmed != clearArmed ||
            lastSetLoop != setLoopEnabled) {
            gridFramebuffer->dirty = true;
            lastGroup = activeGroup;
            lastPreset = activePreset;
            lastChannel = activeChannel;
            lastRunning = running;
            lastCopyActive = copyActive;  
            lastPasteActive = pasteActive;
            lastClearArmed = clearArmed;
            lastSetLoop = setLoopEnabled;
        }
    }
	

        // === PENDING HIGHLIGHT MANAGEMENT ===
        // Keep source highlight (pulsing yellow) until the entire copy/paste operation is complete
        // Only clear everything when user exits all special modes
        bool anyModeActive = copyActive || pasteActive || clearArmed || setLoopEnabled;

        if (!anyModeActive) {
            pendingCopyRow = -1;
            pendingCopyType = NONE;
            pendingPasteRow = -1;
            pendingClearRow = -1;
            pendingClearCol = -1;
        } else {
            // Clear only the hover previews when leaving their respective modes
            if (!pasteActive) pendingPasteRow = -1;
            if (!clearArmed) {
                pendingClearRow = -1;
                pendingClearCol = -1;
            }
            // Do NOT clear pendingCopyRow here — it stays until operation ends
        }
		
        // === SWING MODE ===
        if (swingModeTrigger.process(params[SWING_MODE_PARAM].getValue())) {
            swingGlobalMode = !swingGlobalMode;
        }
       lights[SWING_MODE_LIGHT].setBrightness(swingGlobalMode ? 1.f : 0.f);

        // === SWING TEMPLATES ===
        for (int i = 0; i < 8; i++) {
            if (swingTemplateTriggers[i].process(params[SWING_TEMPLATE_PARAMS + i].getValue())) {
                applySwingTemplate(i);
            }
        }

        // === SWING CASCADE ===
        if (swingCascadePresetTrigger.process(params[SWING_CASCADE_PRESET_PARAM].getValue())) {
            cascadeSwingToPreset();
        }
        if (swingCascadeGroupTrigger.process(params[SWING_CASCADE_GROUP_PARAM].getValue())) {
            cascadeSwingToGroup();
        }

        // === SWING RANDOMIZE ===
        if (swingRandomizeTrigger.process(params[SWING_RANDOMIZE_PARAM].getValue())) {
            randomizeSwing();
        }

        // === NAVIGATION ===
         // === NAVIGATION (only basic +/- navigation now) ===
        if (groupPrevTrigger.process(params[GROUP_PREV_PARAM].getValue())) {
            activeGroup = (activeGroup - 1 + 8) % 8;
        }
        if (groupNextTrigger.process(params[GROUP_NEXT_PARAM].getValue())) {
            activeGroup = (activeGroup + 1) % 8;
        }

        if (presetPrevTrigger.process(params[PRESET_PREV_PARAM].getValue())) {
            if (activePreset == (activePreset - 1 + 8) % 8) {
                presetCascade = !presetCascade;
            } else {
                presetCascade = false;
            }
            activePreset = (activePreset - 1 + 8) % 8;
        }
        if (presetNextTrigger.process(params[PRESET_NEXT_PARAM].getValue())) {
            if (activePreset == (activePreset + 1) % 8) {
                presetCascade = !presetCascade;
            } else {
                presetCascade = false;
            }
            activePreset = (activePreset + 1) % 8;
        }

        if (channelPrevTrigger.process(params[CHANNEL_PREV_PARAM].getValue())) {
            activeChannel = (activeChannel - 1 + 8) % 8;
        }
        if (channelNextTrigger.process(params[CHANNEL_NEXT_PARAM].getValue())) {
            activeChannel = (activeChannel + 1) % 8;
        }


        // === RESET ===
        if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
            currentStep = -1;
            phase = 0.f;
            masterClockPulse = 0.f;
            loopCounter = 0;
            for (int i = 0; i < 8; i++) {
                gatePulseTime[i] = 0.f;
                swingDelay[i] = 0.f;
                pendingGate[i] = false;
            }
        }

        if (!running) return;

		// === CLOCK ADVANCE (ESP32-style) ===
		bool advance = false;

		// External clock has priority
		if (inputs[CLK_INPUT].isConnected()) {
			advance = clockTrigger.process(inputs[CLK_INPUT].getVoltage());
		}
		else {
			float bpm = params[BPM_PARAM].getValue();
			if (bpm < 1.f)
				bpm = 1.f;

			// 16th-note interval
			secondsPerStep = 60.f / bpm / 4.f;

			clockPhase += args.sampleTime;

			if (clockPhase >= secondsPerStep) {
				clockPhase -= secondsPerStep;
				advance = true;
			}
		}


        
        if (advance) {
            if (!inputs[CLK_INPUT].isConnected()) phase -= secondsPerStep;
            currentStep = (currentStep + 1) % 8;
            
            // === LOOP TRACKING ===
            if (currentStep == 0) {
                loopCounter++;
                
                if (activeGroup != previousGroup) {
                    lastGroupChangeLoop = loopCounter;
                    previousGroup = activeGroup;
                }
                if (activePreset != previousPreset) {
                    lastPresetChangeLoop = loopCounter;
                    previousPreset = activePreset;
                }
                
                // === GROUP LOOP ADVANCE ===
                if (groupLoopEnabled) {
                    int nextGroup = (activeGroup + 1) % 8;
                    int attempts = 0;
                    while (attempts < 8 && !groupLoopArray[nextGroup]) {
                        nextGroup = (nextGroup + 1) % 8;
                        attempts++;
                    }
                    if (groupLoopArray[nextGroup]) activeGroup = nextGroup;
                }
                
                // === PRESET LOOP ADVANCE ===
                if (presetLoopEnabled) {
                    int nextPreset = (activePreset + 1) % 8;
                    int attempts = 0;
                    while (attempts < 8 && !presetLoopArray[activeGroup][nextPreset]) {
                        nextPreset = (nextPreset + 1) % 8;
                        attempts++;
                    }
                    if (presetLoopArray[activeGroup][nextPreset]) activePreset = nextPreset;
                }
            }
            
            // Fire master clock pulse (2ms like ESP)
            masterClockPulse = 0.002f;
            outputs[MASTER_CLK_OUTPUT].setVoltage(10.f);
            
            // Schedule gates with swing (all 8 channels)
            for (int ch = 0; ch < 8; ch++) {
                uint8_t state = caveArray[activeGroup][activePreset][ch][currentStep];
                
                if (state > 0 && !muteChannel[ch]) {
                    // Get swing offset
                    int8_t swing = swingGlobalMode ? 
                        swingGlobal[ch] : 
                        swingFlat[activeGroup][activePreset][ch][currentStep];
                    
                    // Convert swing (-50 to +50) to time offset
                    float swingTime = (swing / 100.f) * secondsPerStep;
                    
                    if (swingTime <= 0) {
                        // Negative swing = fire early (immediately)
                        outputs[GATE_OUTPUTS + ch].setVoltage(10.f);
                        gatePulseTime[ch] = (state == 1) ? 0.010f : 0.f; // 10ms for triggers
                    } else {
                        // Positive swing = delay
                        swingDelay[ch] = swingTime;
                        pendingGate[ch] = true;
                    }
                } else {
                    outputs[GATE_OUTPUTS + ch].setVoltage(0.f);
                    gatePulseTime[ch] = 0.f;
                }
            }
        }



        // === SWING DELAY PROCESSING ===
        for (int ch = 0; ch < 8; ch++) {
            if (pendingGate[ch]) {
                swingDelay[ch] -= args.sampleTime;
                if (swingDelay[ch] <= 0.f) {
                    uint8_t state = caveArray[activeGroup][activePreset][ch][currentStep];
                    outputs[GATE_OUTPUTS + ch].setVoltage(10.f);
                    gatePulseTime[ch] = (state == 1) ? 0.010f : 0.f;
                    pendingGate[ch] = false;
                }
            }
        }

        // === MASTER CLOCK PULSE ===
        if (masterClockPulse > 0.f) {
            masterClockPulse -= args.sampleTime;
            if (masterClockPulse <= 0.f) {
                outputs[MASTER_CLK_OUTPUT].setVoltage(0.f);
            }
        }

        // === GATE PULSE SHORTENING (only for triggers, not gates) ===
        for (int ch = 0; ch < 8; ch++) {
            if (gatePulseTime[ch] > 0.f) {
                gatePulseTime[ch] -= args.sampleTime;
                if (gatePulseTime[ch] <= 0.f) {
                    // Only turn off if it's a trigger (state==1), not a gate (state==9)
                    uint8_t state = caveArray[activeGroup][activePreset][ch][currentStep];
                    if (state == 1) {
                        outputs[GATE_OUTPUTS + ch].setVoltage(0.f);
                    }
                }
            }
        }
    
	

	
	
	
	} //END of process code
	


    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        
        json_t* caveJ = json_array();
        for (int g = 0; g < 8; g++) {
            for (int p = 0; p < 8; p++) {
                for (int c = 0; c < 8; c++) {
                    for (int s = 0; s < 8; s++) {
                        json_array_append_new(caveJ, json_integer(caveArray[g][p][c][s]));
                    }
                }
            }
        }
        json_object_set_new(rootJ, "caveArray", caveJ);
        
        json_object_set_new(rootJ, "activeGroup", json_integer(activeGroup));
        json_object_set_new(rootJ, "activePreset", json_integer(activePreset));
        json_object_set_new(rootJ, "activeChannel", json_integer(activeChannel));
        json_object_set_new(rootJ, "viewMode", json_integer(viewMode));
        json_object_set_new(rootJ, "enableESP32Sync", json_boolean(enableESP32Sync));
		json_object_set_new(rootJ, "esp32IP", json_string(esp32IP.c_str()));
		
		json_object_set_new(rootJ, "randomWeight", json_real(randomWeight));
        return rootJ;
    }
	
	
void dataFromJson(json_t* rootJ) override {
        json_t* caveJ = json_object_get(rootJ, "caveArray");
        if (caveJ) {
            int idx = 0;
            for (int g = 0; g < 8; g++) {
                for (int p = 0; p < 8; p++) {
                    for (int c = 0; c < 8; c++) {
                        for (int s = 0; s < 8; s++) {
                            json_t* valJ = json_array_get(caveJ, idx++);
                            if (valJ) caveArray[g][p][c][s] = json_integer_value(valJ);
                        }
                    }
                }
            }
        }
        
        json_t* groupJ = json_object_get(rootJ, "activeGroup");
        if (groupJ) activeGroup = json_integer_value(groupJ);
        // ... etc
		    json_t* syncJ = json_object_get(rootJ, "enableESP32Sync");
    if (syncJ) enableESP32Sync = json_boolean_value(syncJ);
    
    json_t* ipJ = json_object_get(rootJ, "esp32IP");
    if (ipJ) esp32IP = json_string_value(ipJ);
	
	    json_t* rndJ = json_object_get(rootJ, "randomWeight");
    if (rndJ) randomWeight = json_real_value(rndJ);
	
    }


    // === COPY/PASTE FUNCTIONS ===
void copyGroup(int group) {
    for (int preset = 0; preset < 8; preset++) {
        for (int channel = 0; channel < 8; channel++) {
            for (int step = 0; step < 8; step++) {
                groupCopyBuffer[preset][channel][step] = caveArray[group][preset][channel][step];
            }
        }
    }
    // Optional: Copy swing if needed
    // for (int preset = 0; preset < 8; preset++) for (int channel = 0; channel < 8; channel++) for (int step = 0; step < 8; step++) {
    //     swingCopyBuffer[step] = swingFlat[group][preset][channel][step];  // If swing per step
    // }
}

void pasteGroup(int targetGroup) {
    for (int preset = 0; preset < 8; preset++) {
        for (int channel = 0; channel < 8; channel++) {
            for (int step = 0; step < 8; step++) {
                caveArray[targetGroup][preset][channel][step] = groupCopyBuffer[preset][channel][step];
            }
        }
    }
    // Optional: Paste swing
}

void copyPreset(int preset) {
    for (int channel = 0; channel < 8; channel++) {
        for (int step = 0; step < 8; step++) {
            presetCopyBuffer[channel][step] = caveArray[activeGroup][preset][channel][step];
        }
    }
}

void pastePreset(int targetPreset) {
    for (int channel = 0; channel < 8; channel++) {
        for (int step = 0; step < 8; step++) {
            caveArray[activeGroup][targetPreset][channel][step] = presetCopyBuffer[channel][step];
        }
    }
}

void copyChannel(int channel) {
    for (int step = 0; step < 8; step++) {
        channelCopyBuffer[step] = caveArray[activeGroup][activePreset][channel][step];
    }
}

void pasteChannel(int targetChannel) {
    for (int step = 0; step < 8; step++) {
        caveArray[activeGroup][activePreset][targetChannel][step] = channelCopyBuffer[step];
    }
}
    // === CLEAR FUNCTIONS ===
    void clearGroup(int g) {
        memset(caveArray[g], 0, sizeof(caveArray[g]));
    }
    
    void clearPreset(int p) {
        memset(caveArray[activeGroup][p], 0, sizeof(caveArray[activeGroup][p]));
    }
    
    void clearChannel(int c) {
        memset(caveArray[activeGroup][activePreset][c], 0, sizeof(caveArray[activeGroup][activePreset][c]));
    }


    // === Random FUNCTIONS ===
    void randomGroup(int g) {
               for (int preset = 0; preset < 8; preset++) {
            for (int ch = 0; ch < 8; ch++) {
                for (int step = 0; step < 8; step++) {
                    caveArray[g][preset][ch][step] = 
                        (random::uniform() < randomWeight) ? 1 : 0;
                }
            }
        }
    }
	
	
	    void randomPreset(int g) {
        for (int ch = 0; ch < 8; ch++) {
            for (int step = 0; step < 8; step++) {
                caveArray[activeGroup][g][ch][step] = 
                    (random::uniform() < randomWeight) ? 1 : 0;
            }
        }
    }
	
	
		    void randomChannel(int g) {
        for (int step = 0; step < 8; step++) {
            caveArray[activeGroup][activePreset][g][step] = 
                (random::uniform() < randomWeight) ? 1 : 0;
        }
    }
	
	
void applySwingTemplate(int templateIndex) {
    for (int step = 0; step < 8; step++) {
        swingFlat[activeGroup][activePreset][activeChannel][step] = 
            swingTemplates[templateIndex][step];
    }
    swingGlobalMode = false; // Switch to per-step mode
}

void cascadeSwingToPreset() {
    for (int ch = 0; ch < 8; ch++) {
        if (ch != activeChannel) {
            for (int step = 0; step < 8; step++) {
                swingFlat[activeGroup][activePreset][ch][step] = 
                    swingFlat[activeGroup][activePreset][activeChannel][step];
            }
        }
    }
}

void cascadeSwingToGroup() {
    for (int preset = 0; preset < 8; preset++) {
        if (preset != activePreset) {
            for (int step = 0; step < 8; step++) {
                swingFlat[activeGroup][preset][activeChannel][step] = 
                    swingFlat[activeGroup][activePreset][activeChannel][step];
            }
        }
    }
}

void randomizeSwing() {
    int minSwing = -20;
    int maxSwing = 20;
    for (int step = 0; step < 8; step++) {
        int range = maxSwing - minSwing;
        int8_t randomValue = minSwing + (random::uniform() * (range + 1));
        swingFlat[activeGroup][activePreset][activeChannel][step] = randomValue;
    }
}

void copySwing() {
    memcpy(swingCopyBuffer, 
           swingFlat[activeGroup][activePreset][activeChannel], 
           sizeof(swingCopyBuffer));
}

void pasteSwing() {
    memcpy(swingFlat[activeGroup][activePreset][activeChannel],
           swingCopyBuffer, 
           sizeof(swingCopyBuffer));
}

void clearSwing() {
    memset(swingFlat[activeGroup][activePreset][activeChannel], 
           0, 
           sizeof(swingFlat[activeGroup][activePreset][activeChannel]));
}

void saveESPBinary(std::string path);
void loadESPBinary(std::string path);
// When clicking the same preset button twice, toggle cascade mode
// In cascade mode, step edits apply to ALL presets in the group
void cascadeStep(int stepIndex) {
    uint8_t current = caveArray[activeGroup][activePreset][activeChannel][stepIndex];
    uint8_t next = (current == 0) ? 1 : (current == 1 ? 9 : 0);
    
    for (int preset = 0; preset < 8; preset++) {
        caveArray[activeGroup][preset][activeChannel][stepIndex] = next;
    }
}


//new check for esp32 bool status
void checkESP32Connection() {
    SimpleHTTPClient client(esp32IP, 80);
    std::vector<uint8_t> response;
    
    if (client.get("/api/status", response)) {
        // Check if response contains "CAVIAN2"
        std::string resp(response.begin(), response.end());
        esp32Connected = (resp.find("CAVIAN2") != std::string::npos && 
                         resp.find("\"status\":\"ok\"") != std::string::npos);
        
        if (esp32Connected) {
            INFO("ESP32 connection OK");
			fetchESP32Patterns();
        }
    } else {
        esp32Connected = false;
    }
}



// Fetch available patterns from ESP32
void fetchESP32Patterns() {
    SimpleHTTPClient client(esp32IP, 80);
    std::vector<uint8_t> response;
    
    if (!client.get("/api/patterns", response)) {
        WARN("Failed to fetch ESP32 patterns");
        return;
    }
    
std::string rawResponse(response.begin(), response.end());

//INFO("Raw HTTP response (%u bytes):\n%s", 
//     static_cast<unsigned int>(response.size()), rawResponse.c_str());

// Find the end of the headers
size_t bodyPos = rawResponse.find("\r\n\r\n");
if (bodyPos == std::string::npos) {
    WARN("Failed to find end of HTTP headers in ESP32 response");
    return;
}

// Extract everything after the double CRLF
std::string jsonStr = rawResponse.substr(bodyPos + 4);

// Optional: log the clean JSON body for confirmation
INFO("Extracted JSON body (%u bytes): %s", 
     static_cast<unsigned int>(jsonStr.length()), jsonStr.c_str());
	
    json_error_t error;
    json_t* rootJ = json_loads(jsonStr.c_str(), 0, &error);
    
    if (!rootJ) {
        WARN("Failed to parse ESP32 patterns JSON: %s", error.text);
        return;
    }
    
    // Clear existing ESP32 pattern list
    for (int box = 0; box < 8; box++) {
        for (int slot = 0; slot < 8; slot++) {
            esp32PatternNames[box][slot] = "";
        }
    }
    
    // Parse pattern names - format: {"1_1":"pattern 1", "2_1":"another pattern"}
    const char* key;
    json_t* value;
    
    json_object_foreach(rootJ, key, value) {
        if (json_is_string(value)) {
            std::string keyStr(key);
            std::string patternName = json_string_value(value);
            
            // Parse key format "X_Y" where X=box (1-8), Y=slot (1-8)
            size_t underscorePos = keyStr.find('_');
            if (underscorePos != std::string::npos) {
                try {
                    int box = std::stoi(keyStr.substr(0, underscorePos)) - 1;  // Convert to 0-indexed
                    int slot = std::stoi(keyStr.substr(underscorePos + 1)) - 1;
                    
                    if (box >= 0 && box < 8 && slot >= 0 && slot < 8) {
                        esp32PatternNames[box][slot] = patternName;
                 //       INFO("ESP32 pattern: Box %d, Slot %d = '%s'", 
                 //            box + 1, slot + 1, patternName.c_str());
                    }
                } catch (...) {
                    WARN("Failed to parse pattern key: %s", keyStr.c_str());
                }
            }
        }
    }
    
    esp32PatternsLoaded = true;
    
    // Count patterns
    int count = 0;
    for (int box = 0; box < 8; box++) {
        for (int slot = 0; slot < 8; slot++) {
            if (!esp32PatternNames[box][slot].empty()) count++;
        }
    }
    
    INFO("Loaded %d patterns from ESP32", count);
    json_decref(rootJ);
}






    std::string esp32IP = "mycavian.local"; // Default, make configurable in sub menu later?
    bool networkConnected = false;
    
    // Test connection
    bool testConnection() {
        SimpleHTTPClient client(esp32IP, 80);
        std::vector<uint8_t> response;
        
        if (client.get("/api/status", response)) {
            // Check if response contains "CAVIAN2"
            std::string resp(response.begin(), response.end());
            networkConnected = (resp.find("CAVIAN2") != std::string::npos);
            return networkConnected;
        }
        
        networkConnected = false;
        return false;
    }
    
    // Upload current pattern to ESP32 // int slot is actually the combined ouput so 11 or 88
   bool uploadToESP32(int slot, std::string patternName = "") {
        if (!esp32Connected) {
            INFO("Not connected to ESP32");
            return false;
        }
        
		    // Calculate box and slot from combined number (e.g., 21 = box 2, slot 1)
    int slotNum = slot / 10;
    int box = slot % 10; // reversing these as the logic is messed up elsewhere :-)
	
        // Create binary data in memory
        std::vector<uint8_t> data;
        
        // === BUILD BINARY (same as saveESPBinary) ===
        data.push_back(2); // Version
        data.push_back(activeGroup);
        data.push_back(activePreset);
        data.push_back(activeChannel);
        
        int bpm = (int)params[BPM_PARAM].getValue();
        data.push_back((bpm >> 8) & 0xFF);
        data.push_back(bpm & 0xFF);
        
        data.push_back(0); // BEAT_SYNC_MODE NOT YET on VCV
        data.push_back(groupLoopEnabled); // 
        data.push_back(presetLoopEnabled); // 
        

		
        // GROUP_LOOP_ARRAY (8 bytes)
        for (int i = 0; i < 8; i++) {
			data.push_back(groupLoopArray[i] ? 1 : 0);
        }
		
        // PRESET_LOOP_ARRAY (64 bytes)
        
		 for (int i = 0; i < 8; i++) {
	       for (int j = 0; j < 8; j++) {
				data.push_back(presetLoopArray[i][j] ? 0 : 1);
		}
    }
		
		
		
		
        // MUTE_CHANNEL_ARRAY (8 bytes)
        for (int i = 0; i < 8; i++) {
            data.push_back(muteChannel[i] ? 0 : 1);
        }
        
        // swingAmount (8 bytes)
        for (int i = 0; i < 8; i++) {
            data.push_back((uint8_t)swingGlobal[i]);
        }
        
        // swingGlobalMode (1 byte)
        data.push_back(swingGlobalMode ? 1 : 0);
        
        // Count sparse cave data
        uint16_t numCaveSteps = 0;
        for (int g = 0; g < 8; g++)
            for (int p = 0; p < 8; p++)
                for (int c = 0; c < 8; c++)
                    for (int s = 0; s < 8; s++)
                        if (caveArray[g][p][c][s] != 0) numCaveSteps++;
        
        data.push_back((numCaveSteps >> 8) & 0xFF);
        data.push_back(numCaveSteps & 0xFF);
        
        // Write sparse cave data
        for (int g = 0; g < 8; g++) {
            for (int p = 0; p < 8; p++) {
                for (int c = 0; c < 8; c++) {
                    for (int s = 0; s < 8; s++) {
                        uint8_t val = caveArray[g][p][c][s];
                        if (val != 0) {
                            data.push_back(g);
                            data.push_back(p);
                            data.push_back(c);
                            data.push_back(s);
                            data.push_back(val);
                        }
                    }
                }
            }
        }
        
        // Count sparse swing data
        uint16_t numSwingSteps = 0;
        for (int g = 0; g < 8; g++)
            for (int p = 0; p < 8; p++)
                for (int c = 0; c < 8; c++)
                    for (int s = 0; s < 8; s++)
                        if (swingFlat[g][p][c][s] != 0) numSwingSteps++;
        
        data.push_back((numSwingSteps >> 8) & 0xFF);
        data.push_back(numSwingSteps & 0xFF);
        
        // Write sparse swing data
        for (int g = 0; g < 8; g++) {
            for (int p = 0; p < 8; p++) {
                for (int c = 0; c < 8; c++) {
                    for (int s = 0; s < 8; s++) {
                        int8_t val = swingFlat[g][p][c][s];
                        if (val != 0) {
                            data.push_back(g);
                            data.push_back(p);
                            data.push_back(c);
                            data.push_back(s);
                            data.push_back((uint8_t)val);
                        }
                    }
                }
            }
        }
        
        // Send to ESP32
        SimpleHTTPClient client(esp32IP, 80);
        std::string path = "/api/pattern?slot=" + std::to_string(slot);
        
		
		INFO("PUT binary to: %s", path.c_str());
        INFO("Body size: %llu bytes", (unsigned long long)data.size());
        INFO("Sending PUT headers:");
        INFO("PUT %s HTTP/1.1", path.c_str());
        INFO("Host: %s", esp32IP.c_str());  // use esp32IP, not hostname (from client)
        INFO("Content-Type: application/octet-stream");
		INFO("Content-Length: %llu", (unsigned long long)data.size());
        INFO("Connection: close");

		

        bool success = client.put(path, data.data(), data.size());
        
        if (success) {
     INFO("Uploaded pattern %d to ESP32 (%llu bytes)", slot, (unsigned long long)data.size());
        } else {
            WARN("Failed to upload pattern %d to ESP32", slot);
        }
        
		
// === 2. UPLOAD PATTERN NAME (if provided) ===
if (!patternName.empty()) {
	 SimpleHTTPClient nameClient(esp32IP, 80);
    json_t* nameJ = json_object();
    json_object_set_new(nameJ, "slot", json_integer(slotNum));
    json_object_set_new(nameJ, "box", json_integer(box));
    json_object_set_new(nameJ, "name", json_string(patternName.c_str()));

    char* jsonStr = json_dumps(nameJ, JSON_COMPACT);
    json_decref(nameJ);

    if (jsonStr) {
        // Get length safely — json_dumps always null-terminates, but let's be robust
        size_t jsonLen = strlen(jsonStr);

        // Copy data BEFORE freeing
        std::vector<uint8_t> nameData(jsonStr, jsonStr + jsonLen);

        // Now safe to free
        free(jsonStr);

std::string dbg(nameData.begin(), nameData.end());
INFO("Sending pattern name JSON: %s", dbg.c_str());

        bool nameSuccess = nameClient.put("/api/pattern/name",
               nameData.data(),
               nameData.size(),
               "application/json");






        if (nameSuccess) {
            INFO("Uploaded pattern name: '%s' (Box %d, Slot %d)", 
                 patternName.c_str(), box , slotNum);
        } else {
            WARN("Failed to upload pattern name");
        }
    } else {
        WARN("Failed to serialize pattern name to JSON");
    }
}
		
		
		
        return success;
    }
    
    // Download pattern from ESP32
    bool downloadFromESP32(int slot) {
        if (!esp32Connected) {
            INFO("Not connected to ESP32");
            return false;
        }
        
        SimpleHTTPClient client(esp32IP, 80);
        std::vector<uint8_t> response;
        std::string path = "/api/pattern?slot=" + std::to_string(slot);
        
        if (!client.get(path, response)) {
            WARN("Failed to download pattern %d from ESP32", slot);
            return false;
        }
        
        // Find start of binary data (skip HTTP headers)
        size_t dataStart = 0;
        for (size_t i = 0; i < response.size() - 3; i++) {
            if (response[i] == '\r' && response[i+1] == '\n' && 
                response[i+2] == '\r' && response[i+3] == '\n') {
                dataStart = i + 4;
                break;
            }
        }
        
        if (dataStart == 0 || dataStart >= response.size()) {
            WARN("Invalid HTTP response");
            return false;
        }
        
        // Parse binary data (same as loadESPBinary but from memory)
        size_t idx = dataStart;
        
        uint8_t version = response[idx++];
        if (version != 2) {
            WARN("Unsupported version: %d", version);
            return false;
        }
        
        activeGroup = response[idx++];
        activePreset = response[idx++];
        activeChannel = response[idx++];
        
        int bpm = (response[idx] << 8) | response[idx+1];
        idx += 2;
        params[BPM_PARAM].setValue(bpm);
        
        idx += 3; // Skip sync flags
		
		
		// Read GROUP_LOOP_ARRAY
        for (int i = 0; i < 8; i++) {
            groupLoopArray[i] = (response[idx++] == 1);
        }
				
		
		// read PRESET_LOOP_ARRAY (8x8 = 64 bytes)
    for (int i = 0; i < 8; i++) {
	       for (int j = 0; j < 8; j++) {
				presetLoopArray[i][j] = (response[idx++] == 1);
		}
    }
				
	        
        // Read mutes
        for (int i = 0; i < 8; i++) {
            muteChannel[i] = (response[idx++] == 0);
        }
        
        // Read swing global
        for (int i = 0; i < 8; i++) {
            swingGlobal[i] = (int8_t)response[idx++];
        }
        
        swingGlobalMode = (response[idx++] == 1);
        
        // Clear arrays
        memset(caveArray, 0, sizeof(caveArray));
        memset(swingFlat, 0, sizeof(swingFlat));
        
        // Read sparse cave data
        uint16_t numCaveSteps = (response[idx] << 8) | response[idx+1];
        idx += 2;
        
        for (int i = 0; i < numCaveSteps; i++) {
            uint8_t g = response[idx++];
            uint8_t p = response[idx++];
            uint8_t c = response[idx++];
            uint8_t s = response[idx++];
            uint8_t val = response[idx++];
            
            if (g < 8 && p < 8 && c < 8 && s < 8) {
                caveArray[g][p][c][s] = val;
            }
        }
        
        // Read sparse swing data
        uint16_t numSwingSteps = (response[idx] << 8) | response[idx+1];
        idx += 2;
        
        for (int i = 0; i < numSwingSteps; i++) {
            uint8_t g = response[idx++];
            uint8_t p = response[idx++];
            uint8_t c = response[idx++];
            uint8_t s = response[idx++];
            int8_t val = (int8_t)response[idx++];
            
            if (g < 8 && p < 8 && c < 8 && s < 8) {
                swingFlat[g][p][c][s] = val;
            }
        }
        
        INFO("Downloaded pattern %d from ESP32", slot);
        return true;
    }
	
	
	
	// ============================================================================
// PATTERN MANAGEMENT SYSTEM
// Add this section to CavianSequencer.cpp, inside the CavianSequencer struct
// ============================================================================

// Pattern library - 8 boxes × 8 slots - just the names
std::string patternNames[8][8];

// Current pattern info
int currentBox = 0;      // Which box we last saved/loaded from
int currentSlot = 0;     // Which slot we last saved/loaded from
std::string currentPatternName = "Untitled";
bool hasUnsavedChanges = false;

// Get the patterns directory path
std::string getPatternsDir() {
    std::string dir = rack::asset::user("CavianSequencer");
    INFO("Base directory: %s", dir.c_str());
    
    rack::system::createDirectories(dir);
    
    std::string patternsDir = dir + "/patterns";
    INFO("Patterns directory: %s", patternsDir.c_str());
    
    rack::system::createDirectories(patternsDir);
    
    return patternsDir;
}

// Get metadata file path
std::string getMetadataPath() {
    std::string dir = rack::asset::user("CavianSequencer");
    return dir + "/pattern_names.json";
}

// Load pattern library metadata
void loadPatternMetadata() {
    std::string path = getMetadataPath();
    
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        INFO("No pattern metadata found, starting fresh");
        return;
    }
    
    json_error_t error;
    json_t* rootJ = json_loadf(file, 0, &error);
    fclose(file);
    
    if (!rootJ) {
        WARN("Failed to parse pattern metadata: %s", error.text);
        return;
    }
    
    // Parse the metadata
    for (int box = 0; box < 8; box++) {
        std::string boxKey = std::to_string(box + 1);
        json_t* boxJ = json_object_get(rootJ, boxKey.c_str());
        
        if (boxJ) {
            for (int slot = 0; slot < 8; slot++) {
                std::string slotKey = std::to_string(slot + 1);
                json_t* nameJ = json_object_get(boxJ, slotKey.c_str());
                
                if (nameJ && json_is_string(nameJ)) {
                    patternNames[box][slot] = json_string_value(nameJ);
                }
            }
        }
    }
    
    json_decref(rootJ);
    INFO("Loaded pattern metadata");
}

// Save pattern library metadata
void savePatternMetadata() {
    json_t* rootJ = json_object();
    
    for (int box = 0; box < 8; box++) {
        json_t* boxJ = json_object();
        std::string boxKey = std::to_string(box + 1);
        
        for (int slot = 0; slot < 8; slot++) {
            if (!patternNames[box][slot].empty()) {
                std::string slotKey = std::to_string(slot + 1);
                json_object_set_new(boxJ, slotKey.c_str(), 
                    json_string(patternNames[box][slot].c_str()));
            }
        }
        
        json_object_set_new(rootJ, boxKey.c_str(), boxJ);
    }
    
    std::string path = getMetadataPath();
    FILE* file = fopen(path.c_str(), "w");
    if (file) {
        json_dumpf(rootJ, file, JSON_INDENT(2));
        fclose(file);
    }
    
    json_decref(rootJ);
}

// Save pattern to a specific slot
bool savePatternToSlot(int box, int slot, std::string name) {
    if (box < 0 || box >= 8 || slot < 0 || slot >= 8) {
        WARN("Invalid box/slot: %d/%d", box, slot);
        return false;
    }
    
    // Generate filename: SEQ_11.cav to SEQ_88.cav
    std::string filename = string::f("SEQ_%d%d.cav", box + 1, slot + 1);
    std::string path = getPatternsDir() + "/" + filename;
    
    // Save using existing ESP32 binary format
    saveESPBinary(path);
    
    // Update metadata
    patternNames[box][slot] = name;
    
    // Update current pattern info
    currentBox = box;
    currentSlot = slot;
    currentPatternName = name;
    hasUnsavedChanges = false;
    
    savePatternMetadata();
    
    INFO("Saved pattern '%s' to Box %d, Slot %d", name.c_str(), box + 1, slot + 1);
    return true;
}

// Load pattern from a specific slot
bool loadPatternFromSlot(int box, int slot) {
    if (box < 0 || box >= 8 || slot < 0 || slot >= 8) {
        WARN("Invalid box/slot: %d/%d", box, slot);
        return false;
    }
    
    if (patternNames[box][slot].empty()) {
        WARN("Slot is empty: Box %d, Slot %d", box + 1, slot + 1);
        return false;
    }
    
    // Generate filename
    std::string filename = string::f("SEQ_%d%d.cav", box + 1, slot + 1);
    std::string path = getPatternsDir() + "/" + filename;
    
    // Check if file exists
    FILE* testFile = fopen(path.c_str(), "rb");
    if (!testFile) {
        WARN("Pattern file not found: %s", path.c_str());
        return false;
    }
    fclose(testFile);
    
    // Load using existing ESP32 binary format
    loadESPBinary(path);
    
    // Update current pattern info
    currentBox = box;
    currentSlot = slot;
    currentPatternName = patternNames[box][slot];
    hasUnsavedChanges = false;
    
    INFO("Loaded pattern '%s' from Box %d, Slot %d", 
        currentPatternName.c_str(), box + 1, slot + 1);
    return true;
}

// Clear a pattern slot
bool clearPatternSlot(int box, int slot) {
    if (box < 0 || box >= 8 || slot < 0 || slot >= 8) {
        return false;
    }
    
    // Delete the file
    std::string filename = string::f("SEQ_%d%d.cav", box + 1, slot + 1);
    std::string path = getPatternsDir() + "/" + filename;
    
    std::remove(path.c_str());
    
    // Clear metadata
    patternNames[box][slot] = "";
    
    savePatternMetadata();
    
    INFO("Cleared Box %d, Slot %d", box + 1, slot + 1);
    return true;
}

// Mark pattern as modified (call this when user edits anything)
void markPatternModified() {
    hasUnsavedChanges = true;
}

// Initialize pattern system (call in constructor)
void initPatternSystem() {
    // Initialize all slots as empty
    for (int box = 0; box < 8; box++) {
        for (int slot = 0; slot < 8; slot++) {
            patternNames[box][slot] = "";
        }
    }
    
    // Load existing metadata
    loadPatternMetadata();
    
    INFO("Pattern system initialized");
}
	
	
};



struct LoopStatusDisplay : Widget {
    CavianSequencer* module;
    
		LoopStatusDisplay() {
        box.size = mm2px(Vec(30, 10));  
    }
    
    void draw(const DrawArgs& args) override {
        if (!module) return;
        
	
        // Background
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGBA(15, 15, 25, 255));
        nvgFill(args.vg);
        
   std::string loopText = "";
        if (module->groupLoopEnabled) loopText += "G-LOOP: ON ";
        if (module->presetLoopEnabled) loopText += "P-LOOP: ON ";
        if (module->setLoopEnabled) loopText += "SET: ON";
        
        // ADD: Copy/Paste/Clear status labels
        std::string modeText = "";
        if (module->copyActive) modeText += "COPY: ON ";
        if (module->pasteActive) modeText += "PASTE: ON ";
        if (module->clearArmed) modeText += "CLEAR: ON";
        
        // Draw loop text
        nvgFontSize(args.vg, 8);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgText(args.vg, 0, 0, loopText.c_str(), NULL);
        
        // Draw mode text below (or beside - adjust Y position)
        nvgText(args.vg, 0, 10, modeText.c_str(), NULL);  // Offset Y for new line
        
        // Optional: Background/border for better visibility
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgStrokeColor(args.vg, nvgRGBA(100, 100, 100, 255));
        nvgStroke(args.vg);
    }
};



struct SwingDisplay : Widget { // not yet implemented
    CavianSequencer* module;
    
    SwingDisplay() {
        box.size = mm2px(Vec(60, 8));
    }
    
    void draw(const DrawArgs& args) override {
        if (!module) return;
        
        // Background
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGBA(15, 15, 25, 255));
        nvgFill(args.vg);
        
        // Draw swing visualization for current channel
        if (module->swingGlobalMode) {
            // Show global swing value
            int8_t swing = module->swingGlobal[module->activeChannel];
            std::string text = string::f("Global: %+d%%", swing);
            
            nvgFontSize(args.vg, 8);
            nvgFontFaceId(args.vg, APP->window->uiFont->handle);
            nvgFillColor(args.vg, nvgRGBA(16, 185, 129, 255));
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(args.vg, box.size.x / 2, box.size.y / 2, text.c_str(), NULL);
        } else {
            // Show per-step swing pattern as bars
            float barWidth = box.size.x / 8.0f;
            
            for (int step = 0; step < 8; step++) {
                int8_t swing = module->swingFlat[module->activeGroup]
                                              [module->activePreset]
                                              [module->activeChannel]
                                              [step];
                
                float barHeight = (swing / 50.0f) * (box.size.y / 2);
                float barX = step * barWidth;
                float barY = (box.size.y / 2) - barHeight;
                
                nvgBeginPath(args.vg);
                nvgRect(args.vg, barX, barY, barWidth - 1, barHeight);
                
                NVGcolor barColor = swing > 0 ? 
                    nvgRGBA(251, 191, 36, 255) :  // Yellow for positive
                    nvgRGBA(99, 102, 241, 255);    // Indigo for negative
                
                nvgFillColor(args.vg, barColor);
                nvgFill(args.vg);
            }
        }
    }
};





// ============================================================================
// BPM DISPLAY WIDGET - Shows current tempo - not using, clutters the UI
// ============================================================================

struct BPMDisplay : Widget {
    CavianSequencer* module;
    
    BPMDisplay() {
        box.size = mm2px(Vec(10, 4));
    }
    
    void draw(const DrawArgs& args) override {
        // Background
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGBA(15, 15, 25, 255));
        nvgFill(args.vg);
        
        // Border
        nvgStrokeWidth(args.vg, 0.5);
        nvgStrokeColor(args.vg, nvgRGBA(100, 126, 234, 100));
        nvgStroke(args.vg);
        
        // Display BPM value
        if (module) {
            int bpm = (int)module->params[CavianSequencer::BPM_PARAM].getValue();
            std::string text = std::to_string(bpm);
            
            nvgFontSize(args.vg, 10);
            nvgFontFaceId(args.vg, APP->window->uiFont->handle);
            nvgFillColor(args.vg, nvgRGBA(16, 185, 129, 255));
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(args.vg, box.size.x / 2, box.size.y / 2, text.c_str(), NULL);
        } else {
            // Preview mode
            nvgFontSize(args.vg, 10);
            nvgFontFaceId(args.vg, APP->window->uiFont->handle);
            nvgFillColor(args.vg, nvgRGBA(16, 185, 129, 255));
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(args.vg, box.size.x / 2, box.size.y / 2, "120", NULL);
        }
    }
};

struct CustomLabel : Widget {
    CavianSequencer* module;
    std::string labelText;
    
    CustomLabel(std::string text) {
        labelText = text;
        box.size = mm2px(Vec(20, 6));
    }
    
    void draw(const DrawArgs& args) override {
        // Background box
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGBA(30, 30, 50, 255));
        nvgFill(args.vg);
        
        // Border
        nvgStrokeWidth(args.vg, 1.0);
        nvgStrokeColor(args.vg, nvgRGBA(100, 126, 234, 180));
        nvgStroke(args.vg);
        
        // Text
        nvgFontSize(args.vg, 10);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFillColor(args.vg, nvgRGBA(200, 200, 220, 255));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x / 2, box.size.y / 2, labelText.c_str(), NULL);
    }
};

// ============================================================================
// IMPLEMENTATIONS 
// ============================================================================



void ClickableCircleDisplay::draw(const DrawArgs& args) {
    if (!module) return;
    
    float cx = box.size.x / 2.0f;
    float cy = box.size.y / 2.0f;
    float radius = box.size.x / 2.0f;
    
    // === OUTER GLOW (configurable color) ===
   /* NVGcolor glowColor = getGlowColor();
    NVGpaint glowPaint = nvgRadialGradient(args.vg, cx, cy, radius * 0.5f, radius * 1.8f,
        glowColor, nvgRGBA(glowColor.r * 255, glowColor.g * 255, glowColor.b * 255, 0));
    nvgBeginPath(args.vg);
    nvgCircle(args.vg, cx, cy, radius * 1.8f);
    nvgFillPaint(args.vg, glowPaint);
    nvgFill(args.vg);
    */
    // === OUTER SHADOW (gives depth) ===
    if (!pressed) {
        NVGpaint shadowPaint = nvgRadialGradient(args.vg, cx, cy + 1.5f, radius * 0.3f, radius * 1.2f,
            nvgRGBA(0, 0, 0, 80), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, cx, cy + 1.5f, radius);
        nvgFillPaint(args.vg, shadowPaint);
        nvgFill(args.vg);
    }
    
    // === BASE CIRCLE WITH GRADIENT ===
    NVGcolor baseColor = getColor();
    NVGcolor topColor, bottomColor;
    
    if (pressed) {
        // Inverted gradient when pressed
        topColor = nvgRGBA(
            baseColor.r * 0.6f,
            baseColor.g * 0.6f,
            baseColor.b * 0.6f,
            baseColor.a
        );
        bottomColor = nvgRGBA(
            std::min(baseColor.r * 1.2f, 1.0f),
            std::min(baseColor.g * 1.2f, 1.0f),
            std::min(baseColor.b * 1.2f, 1.0f),
            baseColor.a
        );
    } else {
        // Normal gradient
        topColor = nvgRGBA(
            std::min(baseColor.r * 1.5f, 1.0f),  // Increased shine
            std::min(baseColor.g * 1.5f, 1.0f),
            std::min(baseColor.b * 1.5f, 1.0f),
            baseColor.a
        );
        bottomColor = nvgRGBA(
            baseColor.r * 0.6f,
            baseColor.g * 0.6f,
            baseColor.b * 0.6f,
            baseColor.a
        );
    }
    
    float offsetY = pressed ? 1.0f : 0.0f;
    NVGpaint buttonPaint = nvgLinearGradient(args.vg, cx, cy - radius + offsetY, cx, cy + radius + offsetY,
        topColor, bottomColor);
    nvgBeginPath(args.vg);
    nvgCircle(args.vg, cx, cy + offsetY, radius);
    nvgFillPaint(args.vg, buttonPaint);
    nvgFill(args.vg);
    
    // === INNER HIGHLIGHT (extra shiny top) ===
    if (!pressed) {
        NVGpaint highlightPaint = nvgRadialGradient(args.vg, cx, cy - radius * 0.4f, 
            radius * 0.05f, radius * 0.8f,
            nvgRGBA(255, 255, 255, 140), nvgRGBA(255, 255, 255, 0));  // Increased intensity
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, cx, cy, radius);
        nvgFillPaint(args.vg, highlightPaint);
        nvgFill(args.vg);
    }
    
    // === BEVELED EDGE ===
// === BEVELED EDGE ===
nvgStrokeWidth(args.vg, 1.5);

if (!pressed) {
    // Outer rim (top-left) - use getBevelColor()
    nvgBeginPath(args.vg);
    nvgArc(args.vg, cx, cy, radius - 0.5f, nvgDegToRad(135), nvgDegToRad(315), NVG_CW);
    nvgStrokeColor(args.vg, getBevelColor());  // Use virtual method
    nvgStroke(args.vg);
    
    // Inner rim (bottom-right) - stays the same
    nvgBeginPath(args.vg);
    nvgArc(args.vg, cx, cy, radius - 0.5f, nvgDegToRad(315), nvgDegToRad(135), NVG_CW);
    nvgStrokeColor(args.vg, nvgRGBA(0, 0, 0, 120));
    nvgStroke(args.vg);
} else {
    // Pressed state bevels (inverted)
    nvgBeginPath(args.vg);
    nvgArc(args.vg, cx, cy + offsetY, radius - 0.5f, nvgDegToRad(135), nvgDegToRad(315), NVG_CW);
    nvgStrokeColor(args.vg, nvgRGBA(0, 0, 0, 120));
    nvgStroke(args.vg);
    
    nvgBeginPath(args.vg);
    nvgArc(args.vg, cx, cy + offsetY, radius - 0.5f, nvgDegToRad(315), nvgDegToRad(135), NVG_CW);
    nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 100));
    nvgStroke(args.vg);
}
    
    // === OUTER STROKE ===
    nvgStrokeWidth(args.vg, 1.0);
    nvgBeginPath(args.vg);
    nvgCircle(args.vg, cx, cy + offsetY, radius);
    nvgStrokeColor(args.vg, CLR_STROKE_WHITE);
    nvgStroke(args.vg);
    
    // === TEXT WITH SUBTLE SHADOW ===
    nvgFontSize(args.vg, 8);
    nvgFontFaceId(args.vg, APP->window->uiFont->handle);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    
    // Text shadow
    nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 120));
    nvgText(args.vg, cx, cy + offsetY + 0.5f, getText().c_str(), NULL);
    
    // Text foreground
    nvgFillColor(args.vg, getTextColor());
    nvgText(args.vg, cx, cy + offsetY, getText().c_str(), NULL);
}



// ============================================================================
// VIEW MODE DISPLAY
// ============================================================================


//======================================================================
// RUN/STOP DISPLAY
// ============================================================================
struct RunStopDisplay : ClickableCircleDisplay {
    std::string getText() override {
        if (!module) return "START";
        CavianSequencer* m = (CavianSequencer*)module;
        return m->running ? "STOP" : "START";
    }
    
    NVGcolor getColor() override {
        if (!module) return CLR_MUTED;
        CavianSequencer* m = (CavianSequencer*)module;
        return m->running ? CLR_GREEN : CLR_MUTED;
    }
    
    NVGcolor getBevelColor() override {
        // Custom: red when stopped, green when running
        if (!module) return nvgRGBA(255, 80, 80, 180);
        CavianSequencer* m = (CavianSequencer*)module;
        return m->running ? 
            CLR_GREEN :   // Green
            CLR_MUTED;    // Red
    }
    
    void onButton(const event::Button& e) override {
        ClickableCircleDisplay::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (!module) return;
            CavianSequencer* m = (CavianSequencer*)module;
            m->running = !m->running;
            e.consume(this);
        }
    }
};



// === VIEW MODE CIRCLE BUTTONS ===
// These go AFTER the full CavianSequencer struct definition!

struct VerticalViewDisplay : ClickableCircleDisplay {
    std::string getText() override {
        if (!module) return "VERT";
        CavianSequencer* m = (CavianSequencer*)module;
        
        // Display current view mode
        switch (m->viewMode) {
            case CavianSequencer::VERTICAL:
                return "VERT";
            case CavianSequencer::HORIZONTAL_8X8:
                return "8x8";
            case CavianSequencer::HORIZONTAL_64:
                return "1x64";
            default:
                return "VERT";
        }
    }
    
    NVGcolor getColor() override {
        // Always show active color since this is the only view mode button now
        return CLR_VINTAGE_YELLOW;
    }
    
    NVGcolor getBevelColor() override {
        return CLR_VINTAGE_YELLOW;
    }
    
    void onButton(const event::Button& e) override {
        ClickableCircleDisplay::onButton(e); 
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            CavianSequencer* m = (CavianSequencer*)module;
            
            // Cycle through view modes (same as V key)
            m->viewMode = (CavianSequencer::ViewMode)((m->viewMode + 1) % 3);
            
            // Update lights
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 0].setBrightness(
                m->viewMode == CavianSequencer::VERTICAL ? 1.f : 0.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 1].setBrightness(
                m->viewMode == CavianSequencer::HORIZONTAL_8X8 ? 1.f : 0.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 2].setBrightness(
                m->viewMode == CavianSequencer::HORIZONTAL_64 ? 1.f : 0.f);
            
            if (m->gridFramebuffer) {
                m->gridFramebuffer->dirty = true;
            }
            e.consume(this);
        }
    }
};

struct EightByEightViewDisplay : ClickableCircleDisplay {
    std::string getText() override {
        return "8x8";
    }

    NVGcolor getColor() override {
        return (module->viewMode == CavianSequencer::HORIZONTAL_8X8) ? CLR_VINTAGE_YELLOW : CLR_INACTIVE;
    }

    NVGcolor getBevelColor() override {
 return (module->viewMode == CavianSequencer::HORIZONTAL_8X8) ? CLR_VINTAGE_YELLOW : CLR_INACTIVE;
    }


	
	
void onButton(const event::Button& e) override {
	ClickableCircleDisplay::onButton(e); 
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            CavianSequencer* m = (CavianSequencer*)module;
            m->viewMode = CavianSequencer::HORIZONTAL_8X8;
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 0].setBrightness(1.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 1].setBrightness(0.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 2].setBrightness(0.f);
			if (m->gridFramebuffer) {
            m->gridFramebuffer->dirty = true;
        }
            e.consume(this);
        }

    }
};

struct OneBy64ViewDisplay : ClickableCircleDisplay {
    std::string getText() override {
        return "1x64";
    }

    NVGcolor getColor() override {
        return (module->viewMode == CavianSequencer::HORIZONTAL_64) ? CLR_VINTAGE_YELLOW : CLR_INACTIVE;
    }

	  NVGcolor getBevelColor() override {
        return (module->viewMode == CavianSequencer::HORIZONTAL_64) ? CLR_VINTAGE_YELLOW : CLR_INACTIVE;
    }
	
void onButton(const event::Button& e) override {
	ClickableCircleDisplay::onButton(e); 
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            CavianSequencer* m = (CavianSequencer*)module;
            m->viewMode = CavianSequencer::HORIZONTAL_64;
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 0].setBrightness(1.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 1].setBrightness(0.f);
            m->lights[CavianSequencer::VIEW_MODE_LIGHTS + 2].setBrightness(0.f);
			if (m->gridFramebuffer) {
            m->gridFramebuffer->dirty = true;
        }
            e.consume(this);
        }
    }
};
// ======




// ============================================================================
// MAIN DRAW METHOD - Clean and simple
// ============================================================================
void CavianButton::draw(const DrawArgs& args) {
    ButtonState bs = getButtonState();
    NVGcolor bgColor = getButtonColor(bs);

    drawButtonBody(args, bs, bgColor);
   // drawIndicators(args, bs);
    drawLabel(args, bs.label);
    drawHoverEffect(args);
}

// ============================================================================
// BUTTON STATE CALCULATION - FIXED
// ============================================================================
CavianButton::ButtonState CavianButton::getButtonState() {
    ButtonState bs;
    if (!module) return bs;

    CavianSequencer* m = static_cast<CavianSequencer*>(module);
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;

    if (m->viewMode == CavianSequencer::VERTICAL) {
        calculateVerticalModeState(m, row, col, bs);
    }
    else if (m->viewMode == CavianSequencer::HORIZONTAL_8X8) {
        bs.value = m->caveArray[m->activeGroup][m->activePreset][row][col];
        bs.isCurrentStep = (col == m->currentStep && m->running);
    }
    else if (m->viewMode == CavianSequencer::HORIZONTAL_64) {
        bs.value = m->caveArray[m->activeGroup][row][m->activeChannel][col];
        bs.isCurrentStep = (row == m->activePreset && col == m->currentStep && m->running);
    }

    return bs;
}


void CavianButton::calculateVerticalModeState(CavianSequencer* m, int row, int col, ButtonState& bs) {
    bs = ButtonState();  // Clear state

    // Special controls - handle and return early
    if (row == 0) {
        if (col == 5) {
            bs.isLoopControl = true;
            bs.value = m->groupLoopEnabled ? 1 : 0;
            bs.label = "GROUP\nLOOP";
            return;
        } else if (col == 6) {
            bs.isLoopControl = true;
            bs.value = m->setLoopEnabled ? 1 : 0;
            bs.label = "SET\nLOOPS";
            return;
        } else if (col == 7) {
            bs.isLoopControl = true;
            bs.value = m->presetLoopEnabled ? 1 : 0;
            bs.label = "PRESET\nLOOP";
            return;
        }
    }
    if (row == 1) {
        if (col == 5) {
            bs.isCopyPasteButton = true;
            bs.value = m->copyActive ? 1 : 0;
            bs.label = "COPY";
            return;
        } else if (col == 6) {
            bs.isCopyPasteButton = true;
            bs.value = m->pasteActive ? 1 : 0;
            bs.label = "PASTE";
            return;
        } else if (col == 7) {
            bs.isCopyPasteButton = true;
            bs.value = m->clearArmed ? 1 : 0;
            bs.label = "CLEAR";
            return;
        }
    }
	if (row == 2 && col == 5) {
		
		
		    bs.isCopyPasteButton = true;
            bs.value = m->randomArmed ? 1 : 0;
            bs.label = "RANDOM";
		
	}
	

    // Main grid logic for all other buttons
    if (m->setLoopEnabled) {
        if (col == 0) {
            bs.shouldHighlightLoopSelect = m->groupLoopArray[row];
        } else if (col == 1) {
            bs.shouldHighlightLoopSelect = m->presetLoopArray[m->activeGroup][row];
        } else if (col == 2) {
            bs.value = m->caveArray[m->activeGroup][m->activePreset][m->activeChannel][row];
            bs.isCurrentStep = (row == m->currentStep && m->running);
        } else if (col == 3) {
            bs.value = (row == m->activeChannel) ? 1 : 0;
        }
    } else {
        if (col == 0) {
            bs.value = (row == m->activeGroup) ? 1 : 0;
            bs.shouldPulseLoop = m->groupLoopEnabled && m->groupLoopArray[row];
        } else if (col == 1) {
            bs.value = (row == m->activePreset) ? 1 : 0;
            bs.shouldPulseLoop = m->presetLoopEnabled && m->presetLoopArray[m->activeGroup][row];
        } else if (col == 2) {
            bs.value = m->caveArray[m->activeGroup][m->activePreset][m->activeChannel][row];
            bs.isCurrentStep = (row == m->currentStep && m->running);
        } else if (col == 3) {
            bs.value = (row == m->activeChannel) ? 1 : 0;
        } else if (col == 4) {
            bs.isMuteButton = true;
            bs.value = m->muteChannel[row] ? 1 : 0;
        }
    }

    // Set labels for header row (overlays on main grid logic)
    if (row == 0) {
        if (col == 0) bs.label = "GROUPS";
        else if (col == 1) bs.label = "PRESETS";
        else if (col == 2) bs.label = "STEPS";
        else if (col == 3) bs.label = "CHANNEL";
        else if (col == 4) bs.label = "MUTE";
    }

    // COPY/PASTE/CLEAR HIGHLIGHTS (only on target columns)
    if (col == 0 || col == 1 || col == 3) {
        bool isPendingCopy = (m->pendingCopyRow == row) &&
            ((col == 0 && m->pendingCopyType == CavianSequencer::GROUP_COPY) ||
             (col == 1 && m->pendingCopyType == CavianSequencer::PRESET_COPY) ||
             (col == 3 && m->pendingCopyType == CavianSequencer::CHANNEL_COPY));

        bool isPendingPaste = (m->pendingPasteRow == row) &&
            ((col == 0 && m->pendingCopyType == CavianSequencer::GROUP_COPY) ||
             (col == 1 && m->pendingCopyType == CavianSequencer::PRESET_COPY) ||
             (col == 3 && m->pendingCopyType == CavianSequencer::CHANNEL_COPY));

            bool isPendingClear = (m->pendingClearRow == row && m->pendingClearCol == col);

        if (isPendingCopy) {
            bs.shouldPulseLoop = true;  // Pulsing yellow for source
        }
        if (isPendingPaste) {
            bs.shouldHighlightLoopSelect = true;  // Static yellow for target
        }
        if (isPendingClear) {
            bs.shouldPulseRed = true;  // Red pulse for danger
        }
    }
}
// ============================================================================
// COLOR SELECTION
// ============================================================================
NVGcolor CavianButton::getButtonColor(const ButtonState& bs) {
	
	//if (bs.shouldHighlightLoopSelect) {
    //    return bs.value ? CLR_VINTAGE_YELLOW : CLR_MUTED;
   // }
	
    if (bs.isLoopControl) {
        return bs.value ? CLR_VINTAGE_YELLOW : CLR_MUTED;
    }

    if (bs.isMuteButton) {
        return bs.value ? CLR_MUTED : CLR_INACTIVE;
    }

    if (bs.isCopyPasteButton) {
        return bs.value ? CLR_VINTAGE_YELLOW : CLR_INACTIVE;
    }


    if (bs.value == 0) {
        return CLR_INACTIVE;
    }
    else if (bs.value == 1) {
		return CLR_GREEN;

    }
    else {
        return CLR_GATE;
    }
}



// ============================================================================
// DRAWING HELPERS
// ============================================================================
void CavianButton::drawButtonBody(const DrawArgs& args, const ButtonState& bs, NVGcolor bgColor) {
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 1.5f);
    nvgFillColor(args.vg, bgColor);
    nvgFill(args.vg);

/*
    if (bs.value > 0) {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 1.5f, 1.5f, box.size.x - 3, box.size.y - 3, 1.0f);
        nvgFillColor(args.vg, CLR_GLOW_WHITE);
        nvgFill(args.vg);
    }
*/
    if (bs.value == 0) {
        nvgStrokeWidth(args.vg, 0.5f);
        nvgStrokeColor(args.vg, CLR_STROKE_WHITE);
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 1.5f);
        nvgStroke(args.vg);
    }
}

void CavianButton::drawIndicators(const DrawArgs& args, const ButtonState& bs) {

/*
if (bs.isCurrentStep) {
        nvgBeginPath(args.vg);
        // Tighter extension for less clipping risk
        nvgRoundedRect(args.vg, -1.0f, -1.0f, box.size.x + 2.0f, box.size.y + 2.0f, 2.0f);
        nvgStrokeWidth(args.vg, 1.0f);  // Slightly thinner for sharpness
        nvgStrokeColor(args.vg, CLR_MUTED);
        nvgStroke(args.vg);
    }

    if (bs.shouldPulseLoop) {
        float phase = fmodf(system::getTime(), 1.0f);
        float pulse = 0.5f + 0.5f * sinf(phase * 6.28318f * 2.0f);
        NVGcolor pulseColor = nvgRGBA(251, 191, 36, static_cast<int>(255 * pulse));

        nvgStrokeWidth(args.vg, 1.0f);
        nvgStrokeColor(args.vg, pulseColor);
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, -1, -1, box.size.x + 2, box.size.y + 2, 2.0f);
        nvgStroke(args.vg);
    }
	// Static yellow border when selected in Set Loops mode
if (bs.shouldHighlightLoopSelect) {
    nvgStrokeWidth(args.vg, 1.0f);
    nvgStrokeColor(args.vg, CLR_YELLOW);  // Solid amber/yellow
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, -1, -1, box.size.x + 2, box.size.y + 2, 2.0f);
    nvgStroke(args.vg);
}

if (bs.shouldPulseRed) {
    float phase = std::sinf(APP->engine->getSampleTime() * 6.0f);  // Smooth pulse ~3Hz
    float intensity = 0.6f + 0.4f * (phase + 1.0f) / 2.0f;
    NVGcolor redPulse = nvgRGBA(239, 68, 68, static_cast<int>(255 * intensity));
    nvgStrokeColor(args.vg, redPulse);
    nvgStrokeWidth(args.vg, 2.0f);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, -1.5f, -1.5f, box.size.x + 3, box.size.y + 3, 2);
    nvgStroke(args.vg);
}
*/

}

void CavianButton::drawLabel(const DrawArgs& args, const char* label) {
    if (!label) return;

    nvgFontSize(args.vg, 6.0f);
    nvgFontFaceId(args.vg, APP->window->uiFont->handle);
    nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

    std::string text(label);
    size_t nl = text.find('\n');
    if (nl != std::string::npos) {
        std::string line1 = text.substr(0, nl);
        std::string line2 = text.substr(nl + 1);
        float spacing = 3.5f;
        nvgText(args.vg, box.size.x / 2, box.size.y / 2 - spacing, line1.c_str(), nullptr);
        nvgText(args.vg, box.size.x / 2, box.size.y / 2 + spacing, line2.c_str(), nullptr);
    } else {
        nvgText(args.vg, box.size.x / 2, box.size.y / 2, label, nullptr);
    }
}

void CavianButton::drawHoverEffect(const DrawArgs& args) {
    if (APP->event->hoveredWidget == this) {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 1.5f);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 15));
        nvgFill(args.vg);
    }
}


// ============================================================================
// IMPROVED MUTE BUTTON
// ============================================================================

void MuteButton::draw(const DrawArgs& args) {
    bool muted = false;
    if (module) {
        muted = module->muteChannel[channelIndex];
    }
    
    // Color based on mute state
    NVGcolor color = muted ? 
        nvgRGBA(239, 68, 68, 255) :   // Red when muted
        nvgRGBA(60, 60, 80, 255);      // Dark gray when active
    
    // Draw circle
    nvgBeginPath(args.vg);
    nvgCircle(args.vg, box.size.x / 2, box.size.y / 2, box.size.x / 2);
    nvgFillColor(args.vg, color);
    nvgFill(args.vg);
    
    // Border
    nvgStrokeWidth(args.vg, 0.5);
    nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, muted ? 100 : 30));
    nvgStroke(args.vg);
    
    // Draw "M" when muted
    if (muted) {
        nvgFontSize(args.vg, 8);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
      //  nvgText(args.vg, box.size.x / 2, box.size.y / 2, "M", NULL);
    }
    
    // Hover effect
    if (APP->event->hoveredWidget == this) {
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, box.size.x / 2, box.size.y / 2, box.size.x / 2);
        nvgFillColor(args.vg, CLR_STROKE_WHITE);
        nvgFill(args.vg);
    }
}


// ============================================================================
// FIXED BUTTON CLICK HANDLER - Simpler logic

void CavianButton::onButton(const event::Button& e) {
	
	 if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        e.consume(this);
        return;
    }
	
	
    if (e.action != GLFW_PRESS || e.button != GLFW_MOUSE_BUTTON_LEFT || !module) {
        ParamWidget::onButton(e);
        return;
    }

   
	
    CavianSequencer* m = static_cast<CavianSequencer*>(module);
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;

    e.consume(this);

    if (m->viewMode == CavianSequencer::VERTICAL) {
        // TOP-RIGHT CONTROLS (toggle flags, dirty for update)
        if (row <= 2 && col >= 5) {
            if (row == 0) {
                if (col == 5) m->groupLoopEnabled = !m->groupLoopEnabled;
                else if (col == 6) m->setLoopEnabled = !m->setLoopEnabled;
                else if (col == 7) m->presetLoopEnabled = !m->presetLoopEnabled;
            } else if (row == 1) {
                if (col == 5) {
                    m->copyActive = true;
                    m->pasteActive = false;
                    m->clearArmed = false;
                    m->copyType = CavianSequencer::NONE;
                    m->copySourceIndex = -1;
                } else if (col == 6) {
                    if (m->copyType != CavianSequencer::NONE && m->copySourceIndex >= 0) {
                        m->pasteActive = !m->pasteActive;
                    }
                    m->copyActive = false;
                    m->clearArmed = false;
                } else if (col == 7) {
                    m->clearArmed = !m->clearArmed;
                    m->copyActive = false;
                    m->pasteActive = false;
                }
            }
			else if (row == 2 && col == 5){
				m->randomArmed = !m->randomArmed;
			}
			
            if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
			return;
        }

        // TARGET SELECTION MODES
        bool handledByMode = false;

        if (m->setLoopEnabled) {
            if (col == 0) {
                m->groupLoopArray[row] = !m->groupLoopArray[row];
                handledByMode = true;
            } else if (col == 1) {
                m->presetLoopArray[m->activeGroup][row] = !m->presetLoopArray[m->activeGroup][row];
                handledByMode = true;
            }
        } else if (m->copyActive && m->copyType == CavianSequencer::NONE) {
            if (col == 0) {
                m->copyType = CavianSequencer::GROUP_COPY;
                m->copySourceIndex = row;
                m->copyGroup(row);
                m->pendingCopyRow = row;
                m->pendingCopyType = CavianSequencer::GROUP_COPY;
                handledByMode = true;
            } else if (col == 1) {
                m->copyType = CavianSequencer::PRESET_COPY;
                m->copySourceIndex = row;
                m->copyPreset(row);
                m->pendingCopyRow = row;
                m->pendingCopyType = CavianSequencer::PRESET_COPY;
                handledByMode = true;
            } else if (col == 3) {
                m->copyType = CavianSequencer::CHANNEL_COPY;
                m->copySourceIndex = row;
                m->copyChannel(row);
                m->pendingCopyRow = row;
                m->pendingCopyType = CavianSequencer::CHANNEL_COPY;
                handledByMode = true;
            }
            m->copyActive = false;
        } else if (m->pasteActive && m->copyType != CavianSequencer::NONE) {
            if (col == 0 && m->copyType == CavianSequencer::GROUP_COPY) {
                m->pasteGroup(row);
                m->activeGroup = row;
                handledByMode = true;
            } else if (col == 1 && m->copyType == CavianSequencer::PRESET_COPY) {
                m->pastePreset(row);
                m->activePreset = row;
                handledByMode = true;
            } else if (col == 3 && m->copyType == CavianSequencer::CHANNEL_COPY) {
                m->pasteChannel(row);
                m->activeChannel = row;
                handledByMode = true;
            }
            if (handledByMode) {
                m->pasteActive = false;
                m->pendingPasteRow = -1;
                m->pendingCopyRow = -1;
				if (m->gridFramebuffer) {
                    m->gridFramebuffer->dirty = true;  // Already there, but ensure
                }
            }
        } else if (m->clearArmed) {
            if (col == 0) {
                m->clearGroup(row);
           //     if (row == m->activeGroup) m->activeGroup = 0;
                handledByMode = true;
            } else if (col == 1) {
                m->clearPreset(row);
          //      if (row == m->activePreset) m->activePreset = 0;
                handledByMode = true;
            } else if (col == 3) {
                m->clearChannel(row);
          //      if (row == m->activeChannel) m->activeChannel = 0;
                handledByMode = true;
            }
            if (handledByMode) {
                m->clearArmed = false;
                m->pendingClearRow = -1;
            }
        }
		 else if (m->randomArmed) {
            if (col == 0) {
                m->randomGroup(row);
           //     if (row == m->activeGroup) m->activeGroup = 0;
                handledByMode = true;
            } else if (col == 1) {
                m->randomPreset(row);
          //      if (row == m->activePreset) m->activePreset = 0;
                handledByMode = true;
            } else if (col == 3) {
                m->randomChannel(row);
          //      if (row == m->activeChannel) m->activeChannel = 0;
                handledByMode = true;
            }
            if (handledByMode) {
                m->randomArmed = false;
                m->pendingClearRow = -1;
            }
        }
		
		

        if (handledByMode) {
            if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
            return;
        }

        // NORMAL NAVIGATION & EDITING
        if (col == 0) {
            m->activeGroup = row;
        } else if (col == 1) {
            if (row == m->activePreset) {
                m->presetCascade = !m->presetCascade;
            } else {
                m->presetCascade = false;
                m->activePreset = row;
            }
        } else if (col == 2) {
            uint8_t& val = m->caveArray[m->activeGroup][m->activePreset][m->activeChannel][row];
            if (m->presetCascade) {
                m->cascadeStep(row);
            } else {
                val = (val == 0) ? 1 : (val == 1 ? 9 : 0);
            }
        } else if (col == 3) {
            m->activeChannel = row;
        } else if (col == 4) {
            m->muteChannel[row] = !m->muteChannel[row];
        }

        if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
        return;
    }

    // HORIZONTAL MODES (unchanged)
    memset(m->dragVisited, 0, sizeof(m->dragVisited));
    uint8_t* cell = nullptr;
    if (m->viewMode == CavianSequencer::HORIZONTAL_8X8) {
        cell = &m->caveArray[m->activeGroup][m->activePreset][row][col];
    } else if (m->viewMode == CavianSequencer::HORIZONTAL_64) {
        cell = &m->caveArray[m->activeGroup][row][m->activeChannel][col];
    }
    if (cell) {
        *cell = (*cell == 0) ? 1 : (*cell == 1 ? 9 : 0);
        m->dragState = *cell;
        m->isDragging = true;
        m->dragVisited[row][col] = true;
        if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
        APP->event->setDraggedWidget(this, GLFW_MOUSE_BUTTON_LEFT);
    }
}
// ============================================================================
// DRAG HOVER HANDLER - Only apply once per cell
// ============================================================================
void CavianButton::onDragHover(const event::DragHover& e) {
    CavianSequencer* m = static_cast<CavianSequencer*>(module);
    if (!m || !m->isDragging) {
        return;
    }
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;
    if (m->dragVisited[row][col]) {
        return;
    }
    uint8_t* cell = nullptr;
    if (m->viewMode == CavianSequencer::HORIZONTAL_8X8) {
        cell = &m->caveArray[m->activeGroup][m->activePreset][row][col];
    }
    else if (m->viewMode == CavianSequencer::HORIZONTAL_64) {
        cell = &m->caveArray[m->activeGroup][row][m->activeChannel][col];
    }
    if (cell) {
        // Cycle through states: 0 -> 1 -> 9 -> 0 (same logic as initial click)
        *cell = (*cell == 0) ? 1 : (*cell == 1 ? 9 : 0);
        m->dragVisited[row][col] = true;
        if (m->gridFramebuffer) {
            m->gridFramebuffer->dirty = true;
        }
    }
    e.consume(this);
}

void CavianButton::onDragEnd(const event::DragEnd& e) {
    if (!module)
        return;

    // Access the module
    auto* m = static_cast<CavianSequencer*>(module);

    // End drag session
    m->isDragging = false;
    memset(m->dragVisited, 0, sizeof(m->dragVisited));

    e.consume(this);
}



void CavianButton::onEnter(const event::Enter& e) {
    if (!module) return;
    CavianSequencer* m = static_cast<CavianSequencer*>(module);
    int row = buttonIndex / 8;
    int col = buttonIndex % 8;
    if (m->viewMode != CavianSequencer::VERTICAL) return;

    if (m->pasteActive && m->copyType != CavianSequencer::NONE) {
        if ((col == 0 && m->copyType == CavianSequencer::GROUP_COPY) ||
            (col == 1 && m->copyType == CavianSequencer::PRESET_COPY) ||
            (col == 3 && m->copyType == CavianSequencer::CHANNEL_COPY)) {
            m->pendingPasteRow = row;
            if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
        }
    } else if (m->clearArmed) {
        if (col == 0 || col == 1 || col == 3) {
            m->pendingClearRow = row;
			m->pendingClearCol = col;
            if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
        }
    }
    e.consume(this);
}

void CavianButton::onLeave(const event::Leave& e) {
    if (!module) return;
    CavianSequencer* m = static_cast<CavianSequencer*>(module);
    if (m->viewMode != CavianSequencer::VERTICAL) return;

    m->pendingPasteRow = -1;
    m->pendingClearRow = -1;
	m->pendingClearCol = -1;
    if (m->gridFramebuffer) m->gridFramebuffer->dirty = true;
    e.consume(this);
}

void MuteButton::onButton(const ButtonEvent& e) {
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        if (module) {
            module->muteChannel[channelIndex] = !module->muteChannel[channelIndex];
        }
        e.consume(this);
    }
    ParamWidget::onButton(e);
}


// ============================================================================
// IMPROVED NAV DISPLAY
// ============================================================================

void NavDisplay::draw(const DrawArgs& args) {
    // Background
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
    nvgFillColor(args.vg, nvgRGBA(15, 15, 25, 255));
    nvgFill(args.vg);
    
    // Border
    nvgStrokeWidth(args.vg, 0.5);
    nvgStrokeColor(args.vg, nvgRGBA(100, 126, 234, 100));
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
    nvgStroke(args.vg);
    
    // Text
    std::string text = label;
    if (module && valuePtr) {
        text += std::to_string(*valuePtr + 1);
    } else {
        text += "1";
    }
    
    nvgFontSize(args.vg, 9);
    nvgFontFaceId(args.vg, APP->window->uiFont->handle);
    nvgFillColor(args.vg, nvgRGBA(200, 200, 220, 255));
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(args.vg, box.size.x / 2, box.size.y / 2, text.c_str(), NULL);
}




// Save in ESP binary format (matches saveDataToFileBinary exactly)
void CavianSequencer::saveESPBinary(std::string path) {

    FILE* file = fopen(path.c_str(), "wb");
    if (!file) {
        WARN("Cannot open file for writing: %s", path.c_str());
        return;
    }
    
    // === HEADER (matches ESP byte-for-byte) ===
    fputc(2, file);  // Version
    fputc(activeGroup, file);
    fputc(activePreset, file);
    fputc(activeChannel, file);
    
    int bpm = (int)params[BPM_PARAM].getValue();
    fputc((bpm >> 8) & 0xFF, file);
    fputc(bpm & 0xFF, file);
    
    fputc(0, file);  // BEAT_SYNC_MODE (unused in VCV)
    fputc(groupLoopEnabled, file);  // GROUP_LOOPS_ENABLED 
    fputc(presetLoopEnabled, file);  // PRESET_LOOPS_ENABLED 
    
    // === ARRAYS (88 bytes - GROUP_LOOP, PRESET_LOOP, MUTE) ===
    // GROUP_LOOP_ARRAY (8 bytes) - all zeros since VCV doesn't use it
    for (int i = 0; i < 8; i++) {
		fputc(groupLoopArray[i], file);
    }
		

    // PRESET_LOOP_ARRAY (8x8 = 64 bytes) - all zeros
    for (int i = 0; i < 8; i++) {
	       for (int j = 0; j < 8; j++) {
				fputc(presetLoopArray[i][j], file);
		}
    }
	
    // MUTE_CHANNEL_ARRAY (8 bytes)
    for (int i = 0; i < 8; i++) {
        fputc(muteChannel[i] ? 0 : 1, file);  // ESP uses 1=unmuted, 0=muted
    }
    
    // swingAmount (8 bytes as signed int8_t)
    for (int i = 0; i < 8; i++) {
        fputc((uint8_t)swingGlobal[i], file);
    }
    
    // swingGlobalMode (1 byte)
    fputc(swingGlobalMode ? 1 : 0, file);
    
    // === COUNT SPARSE CAVE DATA ===
    uint16_t numCaveSteps = 0;
    for (int g = 0; g < 8; g++)
        for (int p = 0; p < 8; p++)
            for (int c = 0; c < 8; c++)
                for (int s = 0; s < 8; s++)
                    if (caveArray[g][p][c][s] != 0) numCaveSteps++;
    
    fputc((numCaveSteps >> 8) & 0xFF, file);
    fputc(numCaveSteps & 0xFF, file);
    
    // === WRITE SPARSE CAVE DATA ===
    for (int g = 0; g < 8; g++) {
        for (int p = 0; p < 8; p++) {
            for (int c = 0; c < 8; c++) {
                for (int s = 0; s < 8; s++) {
                    uint8_t val = caveArray[g][p][c][s];
                    if (val != 0) {
                        fputc(g, file);
                        fputc(p, file);
                        fputc(c, file);
                        fputc(s, file);
                        fputc(val, file);
                    }
                }
            }
        }
    }
    
    // === COUNT SPARSE SWING DATA ===
    uint16_t numSwingSteps = 0;
    for (int g = 0; g < 8; g++)
        for (int p = 0; p < 8; p++)
            for (int c = 0; c < 8; c++)
                for (int s = 0; s < 8; s++)
                    if (swingFlat[g][p][c][s] != 0) numSwingSteps++;
    
    fputc((numSwingSteps >> 8) & 0xFF, file);
    fputc(numSwingSteps & 0xFF, file);
    
    // === WRITE SPARSE SWING DATA ===
    for (int g = 0; g < 8; g++) {
        for (int p = 0; p < 8; p++) {
            for (int c = 0; c < 8; c++) {
                for (int s = 0; s < 8; s++) {
                    int8_t val = swingFlat[g][p][c][s];
                    if (val != 0) {
                        fputc(g, file);
                        fputc(p, file);
                        fputc(c, file);
                        fputc(s, file);
                        fputc((uint8_t)val, file);
                    }
                }
            }
        }
    }
    
    fclose(file);
    INFO("Saved ESP binary: %d cave + %d swing steps", numCaveSteps, numSwingSteps);
}

// Load from ESP binary format
void CavianSequencer::loadESPBinary(std::string path) {

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        WARN("Cannot open file: %s", path.c_str());
        return;
    }
    
    // === READ HEADER ===
    uint8_t version = fgetc(file);
    if (version != 2) {
        WARN("Unsupported version: %d", version);
        fclose(file);
        return;
    }
    
    activeGroup = fgetc(file);
    activePreset = fgetc(file);
    activeChannel = fgetc(file);
    
    int bpmHigh = fgetc(file);
    int bpmLow = fgetc(file);
    params[BPM_PARAM].setValue((bpmHigh << 8) | bpmLow);
    
    fgetc(file); // BEAT_SYNC_MODE (skip)
    groupLoopEnabled = fgetc(file); // 
    presetLoopEnabled = fgetc(file); // 
    
    // === SKIP GROUP_LOOP_ARRAY (8 bytes) ===
    fseek(file, 8, SEEK_CUR);
    
    // === SKIP PRESET_LOOP_ARRAY (64 bytes) ===
    fseek(file, 64, SEEK_CUR);
    
    // === READ MUTE_CHANNEL_ARRAY (8 bytes) ===
    for (int i = 0; i < 8; i++) {
        uint8_t val = fgetc(file);
        muteChannel[i] = (val == 0);  // ESP: 1=unmuted, 0=muted
    }
    
    // === READ swingAmount (8 bytes) ===
    for (int i = 0; i < 8; i++) {
        swingGlobal[i] = (int8_t)fgetc(file);
    }
    
    // === READ swingGlobalMode ===
    swingGlobalMode = (fgetc(file) == 1);
    
    // === CLEAR ARRAYS ===
    memset(caveArray, 0, sizeof(caveArray));
    memset(swingFlat, 0, sizeof(swingFlat));
    
    // === READ SPARSE CAVE DATA ===
    uint16_t numCaveSteps = (fgetc(file) << 8) | fgetc(file);
    for (int i = 0; i < numCaveSteps; i++) {
        uint8_t g = fgetc(file);
        uint8_t p = fgetc(file);
        uint8_t c = fgetc(file);
        uint8_t s = fgetc(file);
        uint8_t val = fgetc(file);
        
        if (g < 8 && p < 8 && c < 8 && s < 8) {
            caveArray[g][p][c][s] = val;
        }
    }
    
    // === READ SPARSE SWING DATA ===
    uint16_t numSwingSteps = (fgetc(file) << 8) | fgetc(file);
    for (int i = 0; i < numSwingSteps; i++) {
        uint8_t g = fgetc(file);
        uint8_t p = fgetc(file);
        uint8_t c = fgetc(file);
        uint8_t s = fgetc(file);
        int8_t val = (int8_t)fgetc(file);
        
        if (g < 8 && p < 8 && c < 8 && s < 8) {
            swingFlat[g][p][c][s] = val;
        }
    }
    
    fclose(file);
    INFO("Loaded ESP binary: %d cave + %d swing steps", numCaveSteps, numSwingSteps);
}


// ============================================================================
// RIGHT-CLICK MENU SYSTEM
// ============================================================================

// === ESP32 STATUS INDICATOR ===
struct ESP32StatusDisplay : Widget {
    CavianSequencer* module;
    
    ESP32StatusDisplay() {
        box.size = mm2px(Vec(15, 5));
    }
    
    void draw(const DrawArgs& args) override {
        if (!module) return;
        
        // Background
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGBA(15, 15, 25, 255));
        nvgFill(args.vg);
        
        // Status text and color
        std::string statusText;
        NVGcolor statusColor;
        
// In your display code
if (!module->enableESP32Sync) {
    statusText = "No Ext";
    statusColor = nvgRGBA(100, 100, 100, 255); // Gray
} else if (module->esp32Connected) {
    statusText = "Link OK  ";
    statusColor = nvgRGBA(16, 185, 129, 255); // Green
} else if (module->esp32CheckAttempted) {
    // Check was attempted but failed
    statusText = "Not Found  ";
    statusColor = nvgRGBA(239, 68, 68, 255); // Red
} else {
    // Still searching (before first check completes)
    statusText = "Searching  ";
    statusColor = nvgRGBA(251, 191, 36, 255); // Yellow/amber
}
        
        // Border with status color
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgStrokeWidth(args.vg, 1.0);
        nvgStrokeColor(args.vg, statusColor);
        nvgStroke(args.vg);
        
        // Text
        nvgFontSize(args.vg, 7);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFillColor(args.vg, statusColor);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x / 2, box.size.y / 2, statusText.c_str(), NULL);
        
        // Connection indicator dot (only when enabled)
        if (module->enableESP32Sync) {
            float dotX = box.size.x - 6;
            float dotY = box.size.y / 2;
            
            nvgBeginPath(args.vg);
            nvgCircle(args.vg, dotX, dotY, 2.0);
            nvgFillColor(args.vg, statusColor);
            nvgFill(args.vg);
            
            // Pulse effect when connected
            if (module->esp32Connected) {
                float time = fmodf(system::getTime(), 2.0f);
                float pulse = 0.3f + 0.7f * sinf(time * 3.14159f);
                
                nvgBeginPath(args.vg);
                nvgCircle(args.vg, dotX, dotY, 3.0);
                nvgStrokeWidth(args.vg, 1.0);
                nvgStrokeColor(args.vg, nvgRGBA(16, 185, 129, (int)(255 * pulse)));
                nvgStroke(args.vg);
            }
        }
    }
};


// ============================================================================
// PATTERN NAME DIALOG
// ============================================================================
struct PatternNameDialog : ui::TextField {
    CavianSequencer* module;
    int box;
    int slot;
    
    PatternNameDialog(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        placeholder = "Enter pattern name...";
        
        // Pre-fill with existing name if slot has one
        if (!module->patternNames[box][slot].empty()) {
            text = module->patternNames[box][slot];
        }
    }
    
    void onAction(const ActionEvent& e) override {
        if (module && !text.empty()) {
            module->savePatternToSlot(box, slot, text);
        }
        
        // Close the menu
        MenuOverlay* overlay = getAncestorOfType<MenuOverlay>();
        if (overlay) {
            overlay->requestDelete();
        }
    }
};

// ============================================================================
// SAVE PATTERN MENU ITEMS
// ============================================================================
struct SaveToSlotItem : ui::MenuItem {
    CavianSequencer* module;
    int box;
    int slot;
    
    SaveToSlotItem(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        // Show existing name or "Empty"
        std::string slotName = module->patternNames[box][slot].empty() ? 
            "Empty" : module->patternNames[box][slot];
        
        text = string::f("Slot %d: %s", slot + 1, slotName.c_str());
    }
    
    void onAction(const ActionEvent& e) override {
        // Create dialog for pattern name
        PatternNameDialog* nameField = new PatternNameDialog(module, box, slot);
        nameField->setSize(math::Vec(200.f, nameField->getSize().y));

        
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Save Pattern As:"));
        menu->addChild(nameField);
        
        APP->event->setSelectedWidget(nameField);
    }
};

struct SaveToBoxMenu : ui::MenuItem {
    CavianSequencer* module;
    int box;
    
    SaveToBoxMenu(CavianSequencer* m, int b) : module(m), box(b) {
        text = string::f("Box %d", box + 1);
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int slot = 0; slot < 8; slot++) {
            menu->addChild(new SaveToSlotItem(module, box, slot));
        }
        
        return menu;
    }
};

struct SavePatternMenu : ui::MenuItem {
    CavianSequencer* module;
    
    SavePatternMenu(CavianSequencer* m) : module(m) {
        text = "Save Pattern To...";
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int box = 0; box < 8; box++) {
            menu->addChild(new SaveToBoxMenu(module, box));
        }
        
        return menu;
    }
};

// ============================================================================
// LOAD PATTERN MENU ITEMS
// ============================================================================
struct LoadFromSlotItem : ui::MenuItem {
    CavianSequencer* module;
    int box;
    int slot;
    
    LoadFromSlotItem(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        std::string slotName = module->patternNames[box][slot];
        
        if (slotName.empty()) {
            text = string::f("Slot %d: Empty", slot + 1);
            disabled = true;
        } else {
            text = string::f("Slot %d: %s", slot + 1, slotName.c_str());
        }
        
        // Show checkmark if this is the current pattern
        if (module->currentBox == box && module->currentSlot == slot) {
            rightText = CHECKMARK_STRING;
        }
    }
    
    void onAction(const ActionEvent& e) override {
        if (module) {
            // Check for unsaved changes
            if (module->hasUnsavedChanges) {

                    return;
 
            }
            
            module->loadPatternFromSlot(box, slot);
        }
    }
};

struct LoadFromBoxMenu : ui::MenuItem {
    CavianSequencer* module;
    int box;
    
    LoadFromBoxMenu(CavianSequencer* m, int b) : module(m), box(b) {
        text = string::f("Box %d", box + 1);
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int slot = 0; slot < 8; slot++) {
            menu->addChild(new LoadFromSlotItem(module, box, slot));
        }
        
        return menu;
    }
};

struct LoadPatternMenu : ui::MenuItem {
    CavianSequencer* module;
    
    LoadPatternMenu(CavianSequencer* m) : module(m) {
        text = "Load Pattern From...";
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int box = 0; box < 8; box++) {
            menu->addChild(new LoadFromBoxMenu(module, box));
        }
        
        return menu;
    }
};



// ============================================================================
// ESP32 PATTERN MENU ITEMS
// ============================================================================

struct LoadFromESP32SlotItem : ui::MenuItem {
    CavianSequencer* module;
    int box;
    int slot;
    
    LoadFromESP32SlotItem(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        std::string patternName = module->esp32PatternNames[box][slot];
        
        if (patternName.empty()) {
            text = string::f("Slot %d: Empty", slot + 1);
            disabled = true;
        } else {
            text = string::f("Slot %d: %s", slot + 1, patternName.c_str());
        }
    }
    
    void onAction(const ActionEvent& e) override {
        if (module) {
            int combinedNumber = ((box + 1) * 10) + (slot + 1);
            
            INFO("Loading '%s' from ESP32 (Box %d, Slot %d)", 
                 module->esp32PatternNames[box][slot].c_str(), box + 1, slot + 1);
            
            if (module->downloadFromESP32(combinedNumber)) {
                INFO("✓ Pattern loaded from ESP32");
            } else {
                WARN("✗ Failed to load pattern from ESP32");
            }
        }
    }
};

struct LoadFromESP32BoxMenu : ui::MenuItem {
    CavianSequencer* module;
    int box;
    
    LoadFromESP32BoxMenu(CavianSequencer* m, int b) : module(m), box(b) {
        text = string::f("Box %d", box + 1);
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int slot = 0; slot < 8; slot++) {
            menu->addChild(new LoadFromESP32SlotItem(module, box, slot));
        }
        
        return menu;
    }
};

struct LoadFromESP32Menu : ui::MenuItem {
    CavianSequencer* module;
    
    LoadFromESP32Menu(CavianSequencer* m) : module(m) {
        text = "Load from ESP32...";
        rightText = RIGHT_ARROW;
        
        if (!module->esp32Connected || !module->esp32PatternsLoaded) {
            disabled = true;
        }
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int box = 0; box < 8; box++) {
            menu->addChild(new LoadFromESP32BoxMenu(module, box));
        }
        
        return menu;
    }
};


// Add this before SaveToESP32SlotItem
struct ESP32PatternNameDialog : ui::TextField {
    CavianSequencer* module;
    int box;
    int slot;
    
    ESP32PatternNameDialog(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        placeholder = "Enter pattern name...";
        
        // Pre-fill with existing name if available
        if (!module->esp32PatternNames[box][slot].empty()) {
            text = module->esp32PatternNames[box][slot];
        }
    }
    
    void onAction(const ActionEvent& e) override {
        if (module && !text.empty()) {
            int combinedNumber = ((box + 1) * 10) + (slot + 1);
            
            INFO("Uploading '%s' to ESP32 (Box %d, Slot %d)", text.c_str(), box + 1, slot + 1);
            
            if (module->uploadToESP32(combinedNumber, text)) {
                INFO("✓ Pattern uploaded to ESP32");
                // Refresh pattern list
                module->fetchESP32Patterns();
            } else {
                WARN("✗ Failed to upload pattern to ESP32");
            }
        }
        
        // Close menu
        MenuOverlay* overlay = getAncestorOfType<MenuOverlay>();
        if (overlay) {
            overlay->requestDelete();
        }
    }
};


struct SaveToESP32SlotItem : ui::MenuItem {
    CavianSequencer* module;
    int box;
    int slot;
    
    SaveToESP32SlotItem(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        std::string patternName = module->esp32PatternNames[box][slot];
        
        if (patternName.empty()) {
            text = string::f("Slot %d: Empty", slot + 1);
        } else {
            text = string::f("Slot %d: %s (Overwrite)", slot + 1, patternName.c_str());
        }
    }
    
    void onAction(const ActionEvent& e) override {
        if (module) {
            // Create name input dialog
            ESP32PatternNameDialog* nameField = new ESP32PatternNameDialog(module, box, slot);
            nameField->setSize(math::Vec(200.f, nameField->getSize().y));
            
            ui::Menu* menu = createMenu();
            menu->addChild(createMenuLabel("Save to ESP32 as:"));
            menu->addChild(nameField);
            
            APP->event->setSelectedWidget(nameField);
        }
    }
};
struct SaveToESP32BoxMenu : ui::MenuItem {
    CavianSequencer* module;
    int box;
    
    SaveToESP32BoxMenu(CavianSequencer* m, int b) : module(m), box(b) {
        text = string::f("Box %d", box + 1);
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int slot = 0; slot < 8; slot++) {
            menu->addChild(new SaveToESP32SlotItem(module, box, slot));
        }
        
        return menu;
    }
};

struct SaveToESP32Menu : ui::MenuItem {
    CavianSequencer* module;
    
    SaveToESP32Menu(CavianSequencer* m) : module(m) {
        text = "Save to ESP32...";
        rightText = RIGHT_ARROW;
        
        if (!module->esp32Connected) {
            disabled = true;
        }
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int box = 0; box < 8; box++) {
            menu->addChild(new SaveToESP32BoxMenu(module, box));
        }
        
        return menu;
    }
};





// ============================================================================
// MANAGE PATTERNS MENU
// ============================================================================
struct RenamePatternItem : ui::MenuItem {
    CavianSequencer* module;
    
    RenamePatternItem(CavianSequencer* m) : module(m) {
        text = "Rename Current Pattern...";
        
        if (module->currentPatternName.empty() || module->currentPatternName == "Untitled") {
            disabled = true;
        }
    }
    
    void onAction(const ActionEvent& e) override {
        PatternNameDialog* nameField = new PatternNameDialog(
            module, module->currentBox, module->currentSlot);
        nameField->setSize(math::Vec(200.f, nameField->getSize().y));

        
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Rename Pattern:"));
        menu->addChild(nameField);
        
        APP->event->setSelectedWidget(nameField);
    }
};

struct ClearSlotItem : ui::MenuItem {
    CavianSequencer* module;
    int box;
    int slot;
    
    ClearSlotItem(CavianSequencer* m, int b, int s) : module(m), box(b), slot(s) {
        std::string slotName = module->patternNames[box][slot];
        
        if (slotName.empty()) {
            text = string::f("Slot %d: Empty", slot + 1);
            disabled = true;
        } else {
            text = string::f("Slot %d: %s", slot + 1, slotName.c_str());
        }
    }
    
    void onAction(const ActionEvent& e) override {
        std::string slotName = module->patternNames[box][slot];
        std::string msg = string::f("Clear '%s' from Box %d, Slot %d?", 
            slotName.c_str(), box + 1, slot + 1);
        
			Menu* menu = createMenu();
			menu->addChild(createMenuLabel("Confirm"));
			menu->addChild(createMenuItem("Clear slot", "", [=]() {
    // clear logic
	  module->clearPatternSlot(box, slot);
}));

menu->addChild(createMenuItem("Cancel", ""));

    }
};

struct ClearBoxMenu : ui::MenuItem {
    CavianSequencer* module;
    int box;
    
    ClearBoxMenu(CavianSequencer* m, int b) : module(m), box(b) {
        text = string::f("Box %d", box + 1);
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int slot = 0; slot < 8; slot++) {
            menu->addChild(new ClearSlotItem(module, box, slot));
        }
        
        return menu;
    }
};

struct ClearSlotMenu : ui::MenuItem {
    CavianSequencer* module;
    
    ClearSlotMenu(CavianSequencer* m) : module(m) {
        text = "Clear Slot...";
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        for (int box = 0; box < 8; box++) {
            menu->addChild(new ClearBoxMenu(module, box));
        }
        
        return menu;
    }
};

struct ManagePatternsMenu : ui::MenuItem {
    CavianSequencer* module;
    
    ManagePatternsMenu(CavianSequencer* m) : module(m) {
        text = "Manage Patterns";
        rightText = RIGHT_ARROW;
    }
    
    ui::Menu* createChildMenu() override {
        ui::Menu* menu = new ui::Menu;
        
        menu->addChild(new RenamePatternItem(module));
        menu->addChild(new MenuSeparator);
        menu->addChild(new ClearSlotMenu(module));
        
        return menu;
    }
};


struct RandomWeightSlider : ui::Slider {
    CavianSequencer* module;
    
    struct RandomWeightQuantity : Quantity {
        float* valuePtr = nullptr;
        
        void setValue(float value) override {
            if (valuePtr)
                *valuePtr = clamp(value, 0.0f, 1.0f);
        }
        
        float getValue() override {
            return valuePtr ? *valuePtr : 0.5f;
        }
        
        float getMinValue() override { return 0.0f; }
        float getMaxValue() override { return 1.0f; }
        float getDefaultValue() override { return 0.5f; }
        
        std::string getLabel() override { return "Weight"; }
        std::string getUnit() override { return "%"; }
        int getDisplayPrecision() override { return 0; }
        
        float getDisplayValue() override { return getValue() * 100.0f; }
        void setDisplayValue(float v) override { setValue(v / 100.0f); }
		
		    std::string getDisplayValueString() override {
        return string::f("%.0f", getValue() * 100.0f);
    }
	
    };
    
    RandomWeightSlider() {
        box.size.x = 200.0f;
    }
};

// ============================================================================
// CLEAN UI IMPLEMENTATION - Replace your CavianSequencerWidget section
// ============================================================================
struct CavianSequencerWidget : ModuleWidget {
    CavianSequencerWidget(CavianSequencer* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/CavianPanel.svg")));

        // Screws - standard positioning
    //    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    //    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
     //   addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
     //   addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // ===================================================================
        // TOP CONTROLS - Clean row layout
        // ===================================================================
        
        // BPM Knob (larger, more prominent) 
		// RoundBlackKnob
		//Davies1900hBlackKnob
		//Davies1900hWhiteKnob
		//Rogan5PSGray
		//RoundLargeBlackKnob 
		//Rogan6PSWhite
		//Rogan3PSBlue


	 



		/*
        // BPM Display (shows current BPM value)
        BPMDisplay* bpmDisp = new BPMDisplay();
        bpmDisp->module = module;
        bpmDisp->box.pos = mm2px(Vec(7.5, 10));
        addChild(bpmDisp);
        */
		
		// Add to widget:
	//	SwingDisplay* swingDisp = new SwingDisplay();
	//	swingDisp->module = module;
	//	swingDisp->box.pos = mm2px(Vec(8, 118));
	//	addChild(swingDisp);

/*
        // Run Button with Light
        addParam(createParamCentered<LEDBezel>(
            mm2px(Vec(10, 120)), 
            module, 
            CavianSequencer::RUN_PARAM
        ));
        addChild(createLightCentered<LEDBezelLight<GreenLight>>(
            mm2px(Vec(10, 120)), 
            module, 
            CavianSequencer::RUN_LIGHT
        ));
  */
  
        // View Mode Button
		/*
        addParam(createParamCentered<TL1105>(
            mm2px(Vec(80, 120)), 
            module, 
            CavianSequencer::VIEW_MODE_PARAM
        ));
        */
		/*
		OneBy64ViewDisplay* sixtyFourDisp = new OneBy64ViewDisplay();
		sixtyFourDisp->module = module;
		sixtyFourDisp->box.pos = mm2px(Vec(52, 116));
		addChild(sixtyFourDisp);


		EightByEightViewDisplay* eightDisp = new EightByEightViewDisplay();
		eightDisp->module = module;
		eightDisp->box.pos = mm2px(Vec(41, 116));
		addChild(eightDisp);
*/
		//BPM widget
        addParam(createParamCentered<Rogan3PWhite>(
            mm2px(Vec(16.5f, 11.0f)), 
            module, 
            CavianSequencer::BPM_PARAM
        ));
		
		// Run/Stop Display
		if (module){
		RunStopDisplay* runDisp = new RunStopDisplay();
		runDisp->module = module;
		runDisp->box.pos = mm2px(Vec(105, 6));
		addChild(runDisp);
		}

	if (module){
		VerticalViewDisplay* vertDisp = new VerticalViewDisplay();
		vertDisp->module = module;
		vertDisp->box.pos = mm2px(Vec(6, 116));  
		addChild(vertDisp);	
	}
        // ===================================================================
        // BOTTOM SECTION - Inputs and master clock
        
		
		        // Reset Input (bottom left)
        addInput(createInputCentered<ResetInPort>(
            mm2px(Vec(68.5, 121)), 
            module, 
            CavianSequencer::RESET_INPUT
        ));
		
		// Clock Input (bottom left)
        addInput(createInputCentered<ClockInPort>(
            mm2px(Vec(56, 121)), 
            module, 
            CavianSequencer::CLK_INPUT
        ));
           
        // Master Clock Output (bottom right)
        addOutput(createOutputCentered<ClockOutPort>(
            mm2px(Vec(80, 121)), 
            module, 
            CavianSequencer::MASTER_CLK_OUTPUT
        ));
			
				// ESP32 Status Display (bottom right)
					if (module){
		ESP32StatusDisplay* esp32Status = new ESP32StatusDisplay();
		esp32Status->module = module;
		esp32Status->box.pos = mm2px(Vec(102, 118)); // Bottom right area
		addChild(esp32Status);
					}

        // ===================================================================
        // NAVIGATION SECTION - Group/Preset/Channel
        // ===================================================================
        /*
        float navPlusY = 20;
        float navMinusY = 30;
		
        // Group Navigation 
		
		

        addParam(createParamCentered<TL1105>(mm2px(Vec(15, navMinusY)), module, CavianSequencer::GROUP_NEXT_PARAM));
        addParam(createParamCentered<TL1105>(mm2px(Vec(15, navPlusY)), module, CavianSequencer::GROUP_PREV_PARAM));
        NavDisplay* groupDisp = new NavDisplay();
        groupDisp->module = module;
        groupDisp->label = "Group ";
		
		groupDisp->labelOffset = Vec(0, -15);
		
        groupDisp->valuePtr = &module->activeGroup;
        groupDisp->box.pos = mm2px(Vec(8, 23));
        addChild(groupDisp);
        
        // Preset Navigation 
        addParam(createParamCentered<TL1105>(mm2px(Vec(35, navMinusY)), module, CavianSequencer::PRESET_NEXT_PARAM));
        addParam(createParamCentered<TL1105>(mm2px(Vec(35, navPlusY)), module, CavianSequencer::PRESET_PREV_PARAM));
        NavDisplay* presetDisp = new NavDisplay();
        presetDisp->module = module;
        presetDisp->label = "Preset ";
        presetDisp->valuePtr = &module->activePreset;
        presetDisp->box.pos = mm2px(Vec(28, 23));
        addChild(presetDisp);
        
        // Channel Navigation CHANNEL_PREV_PARAM
        addParam(createParamCentered<TL1105>(mm2px(Vec(55, navMinusY)), module, CavianSequencer::CHANNEL_NEXT_PARAM));
        addParam(createParamCentered<TL1105>(mm2px(Vec(55, navPlusY)), module, CavianSequencer::CHANNEL_PREV_PARAM));
        NavDisplay* chanDisp = new NavDisplay();
        chanDisp->module = module;
        chanDisp->label = "Channel ";
        chanDisp->valuePtr = &module->activeChannel;
        chanDisp->box.pos = mm2px(Vec(48, 23));
        addChild(chanDisp);
*/


             // ===================================================================
        // 8x8 STEP GRID - With GridContainer for instant updates and drag handling
        // ===================================================================
// Around line 3910 - SIMPLIFY this entire section:
float gridStartX = 5.0f;
float gridStartY = 22.0f;
float buttonSpacing = 11.5f;

// Padding for glow effects (needs at least 2mm on each side for 2px stroke)
float glowPadding = 0.0f;

// Create container
Widget* gridContainer = new Widget();
gridContainer->box.pos = mm2px(Vec(gridStartX - glowPadding, gridStartY - glowPadding));
gridContainer->box.size = mm2px(Vec(buttonSpacing * 8 + glowPadding * 2, buttonSpacing * 8 + glowPadding * 2));
addChild(gridContainer);

// Framebuffer with same size as container
FramebufferWidget* gridFramebuffer = new FramebufferWidget();
gridFramebuffer->box.pos = Vec(0, 0);
gridFramebuffer->box.size = gridContainer->box.size;
gridFramebuffer->oversample = 2.0f;
gridContainer->addChild(gridFramebuffer);

if (module) {
    module->gridFramebuffer = gridFramebuffer;
}

// Place buttons WITHOUT offset (overlay handles positioning)
for (int i = 0; i < 64; i++) {
    CavianButton* btn = new CavianButton();
    btn->module = module;
    btn->buttonIndex = i;
    int row = i / 8;
    int col = i % 8;
    btn->box.pos = mm2px(Vec(
        glowPadding + col * buttonSpacing,  // Add padding here
        glowPadding + row * buttonSpacing
    ));
    gridFramebuffer->addChild(btn);
}

struct AnimationOverlay : TransparentWidget {
    CavianSequencer* module;
    
    void onButton(const event::Button& e) override {
        // Don't consume - let clicks pass through
    }
    
    void draw(const DrawArgs& args) override {
        if (!module) return;
        
        const float buttonSpacing = 11.5f;
        const float buttonSize = 11.0f;
        
        for (int i = 0; i < 64; i++) {
            int row = i / 8;
            int col = i % 8;
            
            Vec pos = mm2px(Vec(col * buttonSpacing, row * buttonSpacing));
            Vec size = mm2px(Vec(buttonSize, buttonSize));
            
            // Calculate state directly from module data
            bool isCurrentStep = module->isCurrentStep(i);
            bool shouldPulseLoop = module->shouldPulseLoop(i);
            bool shouldHighlightLoop = false;
            
            // Check for loop sevvvlect mode highlighting
            if (module->viewMode == CavianSequencer::VERTICAL && module->setLoopEnabled) {
                if (col == 0) {
                    shouldHighlightLoop = module->groupLoopArray[row];
                } else if (col == 1) {
                    shouldHighlightLoop = module->presetLoopArray[module->activeGroup][row];
                }
            }
            
            // === 1. Current Step ===
            if (isCurrentStep) {
                nvgStrokeWidth(args.vg, 1.0f);
                nvgStrokeColor(args.vg, CLR_BLUE);
                nvgBeginPath(args.vg);
                nvgRoundedRect(args.vg, pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, 2.0f);
                nvgStroke(args.vg);
                continue;
            }
            
            // === 2. Loop pulse (animated) ===
            if (shouldPulseLoop) {
                float phase = fmodf(system::getTime(), 1.0f);
                float pulse = 0.5f + 0.5f * sinf(phase * 6.28318f * 1.0f);
                NVGcolor pulseColor = nvgRGBA(251, 191, 36, static_cast<int>(255 * pulse));
                
                nvgStrokeWidth(args.vg, 1.0f);
                nvgStrokeColor(args.vg, pulseColor);
                nvgBeginPath(args.vg);
                nvgRoundedRect(args.vg, pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, 2.0f);
                nvgStroke(args.vg);
            }
            
            // === 3. Loop select highlight (solid) ===
            else if (shouldHighlightLoop) {
                nvgStrokeWidth(args.vg, 1.0f);
                nvgStrokeColor(args.vg, CLR_YELLOW);
                nvgBeginPath(args.vg);
                nvgRoundedRect(args.vg, pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, 2.0f);
                nvgStroke(args.vg);
            }
        }
    }
};

// Create overlay - no widget pointer needed
AnimationOverlay* overlay = new AnimationOverlay();
overlay->module = module;
overlay->box.pos = Vec(0, 0);
overlay->box.size = gridContainer->box.size;
gridContainer->addChild(overlay);




		// ===================================================================
		// OUTPUT JACKS - Vertical on right side
		// ===================================================================

		float outputX = 110.0f;
		float outputStartY = 30.0f;
		float outputSpacing = 11.0f;
		//float muteOffsetX = -13.0f;  // Distance to the left of the jack (negative = left)

		for (int i = 0; i < 8; i++) {
			addOutput(createOutputCentered<CavianPort>(
				mm2px(Vec(outputX, outputStartY + i * outputSpacing)), 
				module, 
				CavianSequencer::GATE_OUTPUTS + i
			));
			/*
			// Mute buttons to the LEFT of outputs
			MuteButton* mute = new MuteButton();
			mute->module = module;
			mute->channelIndex = i;
			mute->box.pos = mm2px(Vec(outputX + muteOffsetX, outputStartY - 3 + i * outputSpacing));
			addChild(mute);
			*/
		}

// Add after the navigation displays
// remove for now, not really needed?
/*
LoopStatusDisplay* loopStatus = new LoopStatusDisplay();
loopStatus->module = module;
loopStatus->box.pos = mm2px(Vec(20, 115));
addChild(loopStatus);
*/




		
		
		
		
		
		
    } // end of constructor
	
	
	
// In your CavianSequencerWidget class
// In your CavianSequencerWidget class
void onHoverKey(const event::HoverKey& e) override {
    if (e.action == GLFW_PRESS && module) {
        // Cast to your specific module type
        CavianSequencer* cavianModule = dynamic_cast<CavianSequencer*>(module);
        if (!cavianModule) {
            ModuleWidget::onHoverKey(e);
            return;
        }
      
        if (e.key == GLFW_KEY_SPACE) {
            cavianModule->running = !cavianModule->running;
            e.consume(this);
            return;
        }

		// V key: Cycle view mode
		if (e.key == GLFW_KEY_V) {
			// Directly cycle the view mode (same logic as your trigger)
			cavianModule->viewMode = (CavianSequencer::ViewMode)((cavianModule->viewMode + 1) % 3);
												// Force UI update
					if (cavianModule->gridFramebuffer) {
						cavianModule->gridFramebuffer->dirty = true;
					}
			e.consume(this);
			return;
		}
        // Check for number keys 1-8
        if (e.key >= GLFW_KEY_1 && e.key <= GLFW_KEY_8) {
            int index = e.key - GLFW_KEY_1; // Convert key to 0-7 index
            
            // Check which modifier is held
            bool gPressed = (glfwGetKey(APP->window->win, GLFW_KEY_G) == GLFW_PRESS);
            bool pPressed = (glfwGetKey(APP->window->win, GLFW_KEY_P) == GLFW_PRESS);
            bool cPressed = (glfwGetKey(APP->window->win, GLFW_KEY_C) == GLFW_PRESS);
            
            if (gPressed) {
                // G + number: Change group
                cavianModule->activeGroup = index;
                e.consume(this);
                return;
            } else if (pPressed) {
                // P + number: Change preset
                cavianModule->activePreset = index;
                e.consume(this);
                return;
            } else if (cPressed) {
                // C + number: Change channel
                cavianModule->activeChannel = index;
                e.consume(this);
                return;
            } else {
                // No modifier: Toggle mute (existing behavior)
                cavianModule->muteChannel[index] = !cavianModule->muteChannel[index];
				
									// Force UI update
					if (cavianModule->gridFramebuffer) {
						cavianModule->gridFramebuffer->dirty = true;
					}
                e.consume(this);
                return;
            }
        }
    }
    ModuleWidget::onHoverKey(e);
}
	
	// Add this method to CavianSequencerWidget class
void step() override {
    ModuleWidget::step();
    
    CavianSequencer* m = dynamic_cast<CavianSequencer*>(module);
    if (!m) return;
    
    // Force continuous updates when sequencer is running to show current step animation
  //  if (m->running && m->gridFramebuffer) {
  //      m->gridFramebuffer->dirty = true;
  //  }
}


	
	// ============================================================================
// ADD TO CavianSequencerWidget::appendContextMenu()
// ============================================================================

// In your CavianSequencerWidget, add this method:
void appendContextMenu(Menu* menu) override {
    CavianSequencer* module = dynamic_cast<CavianSequencer*>(this->module);
    if (!module){
        return;
    }
	
	
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Random Weighting"));
    
    RandomWeightSlider* slider = new RandomWeightSlider;
    slider->module = module;
    
    RandomWeightSlider::RandomWeightQuantity* q = new RandomWeightSlider::RandomWeightQuantity;
    q->valuePtr = &module->randomWeight;
    slider->quantity = q;
    
    menu->addChild(slider);
	
	menu->addChild(new MenuSeparator);
//    menu->addChild(createMenuLabel("External SYNC"));

    // ← ADD THIS LINE - always visible toggle
    menu->addChild(createBoolPtrMenuItem("Enable Hardware Sync", "", &module->enableESP32Sync));
	
	
	    // === ESP32 SYNC SECTION ===
if (module->enableESP32Sync) {
    std::string statusText = module->esp32Connected ? 
        "✓ Connected to " + module->esp32IP : 
        "✗ Not connected";
    menu->addChild(createMenuLabel(statusText));
    
    if (module->esp32Connected) {
        // Show pattern count
        int patternCount = 0;
        for (int box = 0; box < 8; box++) {
            for (int slot = 0; slot < 8; slot++) {
                if (!module->esp32PatternNames[box][slot].empty()) {
                    patternCount++;
                }
            }
        }
        
        menu->addChild(createMenuLabel(string::f("%d patterns on ESP32", patternCount)));
        
        menu->addChild(new MenuSeparator);
        menu->addChild(new LoadFromESP32Menu(module));
        menu->addChild(new SaveToESP32Menu(module));
        
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Refresh Pattern List", "", [=]() {
        module->fetchESP32Patterns();
        }));
    } else {
        menu->addChild(createMenuItem("Test Connection Now", "", [=]() {
            module->checkESP32Connection();
        }));
    }
}
		
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("PATTERN LIBRARY"));
    
    // Show current pattern info
    if (!module->currentPatternName.empty() && module->currentPatternName != "Untitled") {
        std::string info = string::f("Current: %s (Box %d, Slot %d)%s",
            module->currentPatternName.c_str(),
            module->currentBox + 1,
            module->currentSlot + 1,
            module->hasUnsavedChanges ? " *" : "");
        menu->addChild(createMenuLabel(info));
    } else {
        menu->addChild(createMenuLabel("Current: Unsaved Pattern"));
    }
    
    menu->addChild(new MenuSeparator);
   
    // Main menu items
    menu->addChild(new LoadPatternMenu(module));
    menu->addChild(new SavePatternMenu(module));
    menu->addChild(new MenuSeparator);
    menu->addChild(new ManagePatternsMenu(module));
}
	
		
};


Model* modelCavianSequencer = createModel<CavianSequencer, CavianSequencerWidget>("CavianSequencer");

