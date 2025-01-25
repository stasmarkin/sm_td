Things become really complex, then you press three or more keys. You will need to handle key releases in different order (it might differ from pressing order). Theoretically number of different situations are too large and I'm not able to carefully calculate them right now. However I plan to make it in version 1.1.0.

On other hand, in 99% my cases 3 or more keys pressed always mean, that I want 1st and 2nd key to be held, and 3rd to be tapped (eg. ctrl - alt - del hotkey). There is rare case, when I make finger roll so fast, that a third key pressed even before first is released. But that is a really rare situation in my experience (~ once per 1000 key presses), so forcing hold+hold+tap decision is good enough (for me) once sm_td notice that 3 keys are pressed.

But you might want to know, how exactly does that works and why pressing and releasing is still very responsive. Every state has a macro key and following key. State become active once user press a macro key. That active state will save next pressed key as following key. Once user press third key, that active state is forced to hold stage, when press saved following key and then bypasses third key to be handled somewhere else. In case if following key is another macro key for sm_td, it will create a new state layer (next stack element), so there will be two active states. Here is an example of 4 keys pressing, every of them are sm_td states.

- `↓macro1` creates first state layer for states stack: [0] = { macro_key = macro1, following_key = NULL, stage = TAP }
- `↓macro2` saves macro2 as following key for first state: [0] = { macro_key = macro1, following_key = macro2, stage = FOLLOWING_TAP }
- `↓macro3` changes first state to HOLD, that state send following key tap, so it creates a new state layer, and then new state will register macro3 as following key. So resulting stack will be:
    * [0] = { macro_key = macro1, following_key = macro2, stage = HOLD }
    * [1] = { macro_key = macro2, following_key = macro3, stage = FOLLOWING_TAP }
- `↓macro4` is bypassed with first state (because it is in HOLD stage) and handled with second state. Same as previous press, it changes second state to HOLD, that state send following key tap (macro3), so it creates a new state layer, and then new state will register macro4 as following key. So resulting stack will be:
    * [0] = { macro_key = macro1, following_key = macro2, stage = HOLD }
    * [1] = { macro_key = macro2, following_key = macro3, stage = HOLD }
    * [2] = { macro_key = macro3, following_key = macro4, stage = FOLLOWING_TAP }

As user will release keys, states stack will shrink back to 0 size
		