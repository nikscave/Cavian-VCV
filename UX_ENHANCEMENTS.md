# Cavian Sequencer v2 - UX Enhancement Analysis

## Current Branch: cavianV2

---

## 1. GATE VS TRIGGER TOGGLE

**Current Behavior:**
- Steps cycle: OFF (0) → TRIGGER (1) → GATE (9) → OFF (0)
- Requires 3 clicks to get back to OFF
- Users who want triggers must click twice (OFF→TRIGGER→GATE)

**UX Issue:**
- Most users likely prefer either triggers OR gates, not both
- Extra clicks to cycle through unwanted states

**Proposed Solutions:**
- **Option A**: Right-click on step to toggle between "Trigger Mode" and "Gate Mode" globally
- **Option B**: Add a config submenu (right-click module) with "Default Step Type" preference (Trigger/Gate)
- **Option C**: Hold Shift + click = trigger, regular click = gate (or vice versa)

---

## 2. NAVIGATION (Group/Preset/Channel)

**Current Behavior:**
- Separate +/- buttons for each (▲/▼ arrows)
- Takes up visible panel space
- 6 buttons total for navigation

**UX Issues:**
- Cluttered UI with many small buttons
- Navigation takes space from the grid

**Proposed Solutions:**
- Add encoder knobs for navigation (like BPM)
- Use right-click context menu for jump-to specific group/preset
- Double-tap arrow to go to 0, or long-press to reset

---

## 3. SWING CONTROLS

**Current Behavior:**
- 8 template buttons visible in vertical view
- Global swing knob + 8 individual step swing params
- Cascade buttons for preset/group

**UX Issues:**
- Swing is complex - most users don't need all 8 templates visible
- Takes valuable grid space

**Proposed Solutions:**
- Move swing templates to right-click context menu
- Only show Global Swing knob by default
- "Advanced Swing" toggle in context menu reveals per-step controls

---

## 4. LOOP MODE CONTROLS

**Current Behavior:**
- 3 loop mode buttons in top rows (Group Loop, Set Loops, Preset Loop)
- Set Loop mode allows interactive loop point setting

**UX Issue:**
- Loop controls compete with step buttons in vertical view
- May be confusing which mode is active

**Proposed Solutions:**
- Move loop controls to context menu
- Add clear visual indicator (different color) when in Set Loop mode
- Add "Quick Loop" - hold a step button to set as loop endpoint

---

## 5. COPY/PASTE/CLEAR

**Current Behavior:**
- 3 buttons in row 1, columns 5-7
- Multi-step workflow: Click Copy → select source → Click Paste → select target

**UX Issues:**
- Complex workflow for simple operations
- Buttons take grid space

**Proposed Solutions:**
- Right-click on row/column for context menu with Copy/Paste/Clear
- Drag-and-drop support for copying patterns
- Add "Duplicate" quick action (copy to next)

---

## 6. NO RIGHT-CLICK CONTEXT MENU

**Current Behavior:**
- No context menu implemented currently

**UX Issues:**
- Can't access settings/preferences
- No way to configure module behavior

**Proposed Solutions:**
- Implement `appendContextMenu()` to add:
  - Default gate/trigger preference
  - View mode defaults
  - BPM range settings
  - ESP32 connection settings
  - Reset to defaults option

---

## 7. MUTE BUTTONS

**Current Behavior:**
- 8 small mute buttons on right side of each channel

**UX Issue:**
- Small targets, may be hard to hit

**Proposed Solutions:**
- Add keyboard shortcuts (1-8) to mute/unmute
- Make mute buttons larger
- Show muted state more prominently on step buttons

---

## 8. PRESET CASCADE

**Current Behavior:**
- Toggling preset advances to next and enables cascade mode
- Visual feedback unclear

**UX Issue:**
- Not obvious what's happening when preset cascades

**Proposed Solutions:**
- Clearer visual indicator for cascade mode
- Add "Auto-advance" toggle in context menu

---

## 9. RANDOM BUTTON

**Current Behavior:**
- Single random button (row 2, col 5)
- Uses 50% probability

**UX Issues:**
- No way to adjust probability
- What does random actually do? (affects current channel/preset)

**Proposed Solutions:**
- Right-click on random button to set probability (10%-90%)
- Option to randomize: current channel, all channels in preset, all channels in group

---

## 10. MISSING FEATURES COMPARED TO ESP32

**Potential Additions:**
- Pattern naming (ESP32 has `esp32PatternNames[g][p]`)
- Save/Load to file more easily
- MIDI input support
- CV input for step voltage

---

## Priority Recommendations

### High Priority (Quick Wins)
1. **Add right-click context menu** - Enables many other features
2. **Gate/Trigger preference** - Reduces clicks for majority use case
3. **Hide swing templates by default** - Clean up UI

### Medium Priority
4. Improved mute button UX
5. Copy/Paste context menus
6. Visual loop mode indicator

### Lower Priority
7. Navigation improvements
8. Random probability setting
9. Preset cascade clarity