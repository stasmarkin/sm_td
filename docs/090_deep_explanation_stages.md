All the decision making based on a state machine. There are independent instances of that state machine for each macro key registered in sm_td.
Every state starts with stage called SMTD_STAGE_NONE. There are 6 different stages and each stage reacts differently on user events. That events are pressing and releasing macro key, pressing and releasing another key (called following key), pressing and releasing third key (after registering following key) and waiting for some timeouts. Let me describe each stage, it's prerequisites and reaction on events.


## SMTD_STAGE_NONE

That's initial state. No previous actions are registered and takes into account.

* On waiting nothing will happen.
* On macro key press sm_td block further execution and move to stage SMTD_STAGE_TOUCH.
* On macro key release event will be bypassed.
* On other key press event will be bypassed.
* On other key release event will be bypassed.

## SMTD_STAGE_TOUCH

On this stage we know, that macro key just has been pressed. We don't know whether it's going be a hold, tap or sequence of multiple taps. on_smtd_action(macro, SMTD_ACTION_TOUCH, 0) is executed.

* On waiting stage will be change to SMTD_STAGE_HOLD.
* On macro key press event will be bypassed (since it's not expected to be pressed again).
* On macro key release stage will be changed to SMTD_STAGE_SEQUENCE. It will be assumed as sequential taps, and depending on SMTD_FEATURE_AGGREGATE_TAPS  action SMTD_ACTION_TAP will be executed now, or in the end of stage SMTD_STAGE_SEQUENCE.
* On other key press stage will be changed to SMTD_STAGE_FOLLOWING_TOUCH. That other key will be saved as following key.
* On other key release event will be bypassed.

## SMTD_STAGE_SEQUENCE

On this stage we know, that a physical tap key just have been performed. We need to wait a little to see if there other sequential taps are going to happen.

* On waiting stage will be change to SMTD_STAGE_NONE and tap_count will be reset. Action SMTD_ACTION_TAP will be executed If SMTD_FEATURE_AGGREGATE_TAPS is true.
* On macro key press we assume, that it's sequential tap, so stage is changing to SMTD_STAGE_TOUCH.
* On macro key release is bypassed, as we don't expect macro key to be in pressed position.
* On other key press same actions are performed as on waiting.
* On other key release event will be bypassed.



## SMTD_STAGE_FOLLOWING_TOUCH,

On this stage we have macro and following keys pressed. State machine is not sure yet how to interpret this (tap+tap or hold+tap).

* On waiting stage will be change to SMTD_STAGE_HOLD. Action SMTD_ACTION_HOLD will be executed and then sm_td will send following key press to quantum.
* On macro key press nothing will happen since macro is already assumed as pressed.
* On macro key release stage moves to SMTD_STAGE_TOUCH_RELEASE stage. sm_td still can interpret this.
* On following key press nothing will happen since following key is already assumed as pressed.
* On following key release stage will change to SMTD_STAGE_HOLD. Action SMTD_ACTION_HOLD will be executed and then sm_td will send following key press and release to quantum.
* On other key press stage will be changed to SMTD_STAGE_HOLD. Action SMTD_ACTION_HOLD will be executed and then sm_td will send following key press to quantum.
* On other key release event will be bypassed.


## SMTD_STAGE_HOLD

On this stage we know, that macro key is press long enough to interpret this as HOLD. So no other event will change that stage unless user will release that macro key.

* On waiting nothing will happen.
* On macro key nothing will happen.
* On macro key release action SMTD_ACTION_RELEASE will be fired. Stage will change to SMTD_STAGE_NONE.
* On other key press or release event will be bypassed.

## SMTD_STAGE_TOUCH_RELEASE

On this stage we have macro key just release and following key is still being pressed. If user will release following key quick enough, it will be interpret as hold+tap.


* On waiting stage will be change to SMTD_STAGE_NONE. sm_td will fire SMTD_ACTION_TAP and then send following key press (as it is already physically pressed).
* On macro key press again sm_td will fire SMTD_ACTION_TAP and then send following key press. Stage will change to SMTD_STAGE_TOUCH with empty following key. Currently pressed following key is no longer counts in next macro key press situation.
* On macro key release nothing will happen since macro key is already assumed to be released.
* On following key press nothing will happen since following key is already assumed to be pressed.
* On following key release sm_td interprets whole thing as hold+tap action, so it fires SMTD_ACTION_HOLD event, then send following key press and release to quantum and fires SMTD_ACTION_RELEASE event. Stage will change to SMTD_STAGE_NONE.
* On other key press sm_td will interpret whole situation as tap+tap. So sm_td will fire SMTD_ACTION_TAP action, send following key press and release and then bypassed this other key press to next handlers.
* On other key release event will be bypassed.