Every macro key remain in stage SMTD_STAGE_NONE until it got hit.

Once you hit a marco key, it's stage move to SMTD_STAGE_TOUCH. That is a stage of uncertanty, whether state machine doesn't know if it's going to be tapped or held.
If there is no other key pressing while the macro key is pressed, there are just two options to happen:
- (fig.1) you will be pressing that key for TAP_TERM millis, so state machine will consider this as being held. State machine will go to stage SMTD_STAGE_HOLD and will fire action SMTD_ACTION_HOLD. Once key will be released, state machine will go to stage SMTD_STAGE_NONE and fire action SMTD_ACTION_RELEASE. Sequential tap count will reset to 0.
- (fig.2) you will release a key before a state machine register a hold action (before TAP_TERM millis pass), so state machine will consider that a tap happened. It will go to stage SMTD_STAGE_SEQUENCE and fire action SMTD_ACTION_TAP. SMTD_STAGE_SEQUENCE is an auxiliary stage, that holds a counter of sequential taps. If you tap ant other key or wait for SEQUENCE_TERM millis tap counter will reset to 0 and a stage will be changed back to SMTD_STAGE_NONE

See schemas below to understand all stages movements.
Time on a left, diagram of pressing and releasing keys in a center, and stage with actions on a right. So, time goes from top to bottom.


```
fig.1 Tap and hold 

				 SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - -
                       │macro│   SMTD_STAGE_TOUCH
                       │ key │   on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │
                       │     │
                       │     │
10ms - - - - - - - - - └—————┘ - - - - - - - - - - - - - -
                                 SMTD_STAGE_SEQUENCE
                                 on_smtd_action(macro_key, SMTD_ACTION_TAP, 0)

20ms - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - -
                       │macro│   SMTD_STAGE_TOUCH
                       │ key │   on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 1)
                       │     │
                       │     │
                       │     │
20ms + TAP_TERM  - - - │ - - │ - - - - - - - - - - - - - - 
                       │     │   SMTD_STAGE_HOLD
                       │     │   on_smtd_action(macro_key, SMTD_ACTION_HOLD, 1)
                       │     │
                       │     │
                       │     │
                       │     │
500ms  - - - - - - - - └—————┘ - - - - - - - - - - - - - -
                                 SMTD_STAGE_NONE   
                                 on_smtd_action(macro_key, SMTD_ACTION_RELEASE, 1)                                                            
```




```
fig.2 Sequential taps

				 SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - -
                       │macro│   SMTD_STAGE_TOUCH
                       │ key │   on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │
                       │     │
                       │     │
10ms - - - - - - - - - └—————┘ - - - - - - - - - - - - - -
                                 SMTD_STAGE_SEQUENCE
                                 on_smtd_action(macro_key, SMTD_ACTION_TAP, 0)

20ms - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - -
                       │macro│   SMTD_STAGE_TOUCH
                       │ key │   on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 1)
                       │     │
                       │     │
                       │     │
30ms - - - - - - - - - └—————┘ - - - - - - - - - - - - - -  
                                 SMTD_STAGE_SEQUENCE
                                 on_smtd_action(macro_key, SMTD_ACTION_TAP, 1)

30ms + SEQUENCE_TERM - - - - - - - - - - - - - - - - - - -
                                 SMTD_STAGE_NONE                                                               
```


