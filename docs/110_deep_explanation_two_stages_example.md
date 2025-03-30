Let's introduce some definitions. The following key is a key you press right after a macro key was pressed. There are many variations when following key might be pressed, so different actions will be performed in that situations. Let's explore them one by one.


## Holding macro key

This is the most simple example. If you press macro key long enough, the stage will become SMTD_STAGE_HOLD. In that stage any other key pressing or releasing is not handled by state machine and bypassed to the rest of users and quantum code. You may think about SMTD_STAGE_HOLD as a normal shift key on generic keyboard. If you hold it, no other key will affect it's state, and all other key are pressed and released at the exact time you press or release them.

```
                                        SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - - - - - - - - -
                       │macro│          SMTD_STAGE_TOUCH
                       │ key │          on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │              
                       │     │              
                       │     │              
                       │     │              
                       │     │              
                       │     │              
                       │     │              
TAP_TERM - - - - - - - │ - - │ - - - - - - - - - - - - - - 
                       │     │          SMTD_STAGE_HOLD
                       │     │          on_smtd_action(macro_key, SMTD_ACTION_HOLD, 0)
                       │     │              
1000ms - - - - - - - - │ - - │ ┌—————┐ - - - - - - - - - - - - - - - - -
                       │     │ │foll.│  press(following_key)
                       │     │ │ key │
                       │     │ │     │
1020ms - - - - - - - - │ - - │ └—————┘ - - - - - - - - - - - - - - - - -
                       │     │          release(following_key)
                       │     │ 
                       │     │ 
2000ms - - - - - - - - └—————┘ - - - - - - - - - - - - - - - - - - - - -
                                        SMTD_STAGE_NONE   
                                        on_smtd_action(macro_key, SMTD_ACTION_RELEASE, 0)                                                            
```


## Quick tap while holding macro key

Thing become a little more complex here. If you press following key while stage SMTD_STAGE_TOUCH, the state machine would be sure if your intentions are to hold macro and press fk or tap macro and press fk. So right after you press fk stage would become SMTD_STAGE_FOLLOWING_TOUCH, it does nothing but wait for following user actions. If you release following key quick enough (within FOLLOWING_TAP_TERM millis), state machine would interpret this as hold macro + tap fk, so it will fire SMTD_ACTION_HOLD and tap following key (press and release). And macro key will remain held untill you release it. On release stage will move to SMTD_STAGE_NONE and SMTD_ACTION_RELEASE will be fired.

```
                                        SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - - - - - - - - -
                       │macro│          SMTD_STAGE_TOUCH
                       │ key │          on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │              
10ms - - - - - - - - - │ - - │ ┌—————┐ - - - - - - - - - - - - - - - - -
                       │     │ │foll.│  SMTD_STAGE_FOLLOWING_TOUCH            
                       │     │ │ key │
                       │     │ │     │
20ms - - - - - - - - - │ - - │ └—————┘ - - - - - - - - - - - - - - - - -
                       │     │          SMTD_STAGE_HOLD
                       │     │          on_smtd_action(macro_key, SMTD_ACTION_HOLD, 0)
                       │     │          press(following_key) and release(following_key)
                       │     │ 
                       │     │ 
                       │     │ 
                       │     │ 
2000ms - - - - - - - - └—————┘ - - - - - - - - - - - - - - - - - - - - -
                                        SMTD_STAGE_NONE   
                                        on_smtd_action(macro_key, SMTD_ACTION_RELEASE, 0)                                                            
```


## Long press while holding macro key

Another option of resolving stage SMTD_STAGE_FOLLOWING_TOUCH is to wait for FOLLOWING_TAP_TERM millis. In that case state machine will decide that your intention is to hold both macro and following key together. So after holding both keys long enough stage would be come SMTD_STAGE_HOLD, SMTD_ACTION_HOLD will be fired, and a following key press (without release) will be sent back to quantum. In that state macro key and following key become "decouped", they no longer affect each other, so you may release them in any order. The corresponding key releases will be fired too.


```
                                        SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - - - - - - - - -
                       │macro│          SMTD_STAGE_TOUCH
                       │ key │          on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │              
10ms - - - - - - - - - │ - - │ ┌—————┐ - - - - - - - - - - - - - - - - -
                       │     │ │foll.│  SMTD_STAGE_FOLLOWING_TOUCH            
                       │     │ │ key │  
                       │     │ │     │
                       │     │ │     │
                       │     │ │     │
                       │     │ │     │
                       │     │ │     │
                       │     │ │     │
                       │     │ │     │
10ms +                 │     │ │     │
FOLLOWING_TAP_TERM - - │ - - │ │ - - │ - - - - - - - - - - - - - - - - -
                       │     │ │     │  SMTD_STAGE_HOLD
                       │     │ │     │  on_smtd_action(macro_key, SMTD_ACTION_HOLD, 0)
                       │     │ │     │  press(following_key)
                       │     │ │     │
1000ms - - - - - - - - │     │ └—————┘ - - - - - - - - - - - - - - - - -
                       │     │          release(following_key)
                       │     │          
                       │     │ 
2000ms - - - - - - - - └—————┘ - - - - - - - - - - - - - - - - - - - - -
                                        SMTD_STAGE_NONE   
                                        on_smtd_action(macro_key, SMTD_ACTION_RELEASE, 0)                                                            
```


## Almost simultaneous release

The third option of resolving stage SMTD_STAGE_FOLLOWING_TOUCH is releasing macro key before it become interpreted as held. In that case following key is still being pressed and state machine is not yet able to make a decision if macro was held or tapped. So state machine goes to stage SMTD_STAGE_TOUCH_RELEASE and just wait for next user input. In case if you release following key fast enough (within RELEASE_TERM millis) it would be counted the same way as it was released while pressing macro key. So stage will go to SMTD_STAGE_NONE and following action will be performed: fire SMTD_ACTION_HOLD, tap following key and fire SMTD_ACTION_RELEASE.


```
                                        SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - - - - - - - - -
                       │macro│          SMTD_STAGE_TOUCH
                       │ key │          on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │              
10ms - - - - - - - - - │ - - │ ┌—————┐ - - - - - - - - - - - - - - - - -
                       │     │ │foll.│  SMTD_STAGE_FOLLOWING_TOUCH            
                       │     │ │ key │  
                       │     │ │     │
20ms - - - - - - - - - └—————┘ │ - - │ - - - - - - - - - - - - - - - - -
                               │     │  SMTD_STAGE_TOUCH_RELEASE
                               │     │
30ms - - - - - - - - - - - - - └—————┘ - - - - - - - - - - - - - - - - -
                                        SMTD_STAGE_NONE
                                        on_smtd_action(macro_key, SMTD_ACTION_HOLD , 0)
                                        press(following_key) and release(following_key)
                                        on_smtd_action(macro_key, SMTD_ACTION_RELEASE, 0)
```


## Release following key much later after tapping macro key

Let's see what would happed, if you continue holding following key on SMTD_STAGE_TOUCH_RELEASE. Macro key is already released, but state machine hasn't make a decision whether it was a tap or hold. So, holding following key for long time (RELEASE_TERM millis) will lead to sequential taps decision. So after RELEASE_TERM state machine will go to stage SMTD_STAGE_NONE, fire action SMTD_ACTION_TAP and press (without releasing) following key. Please, note that sending press event of following key to OS happens much later than user physically press a key, so sm_td "delays" that press until it is absolutely sure how to interpret this.

```
                                        SMTD_STAGE_NONE
0ms  - - - - - - - - - ┌—————┐ - - - - - - - - - - - - - - - - - - - - -
                       │macro│          SMTD_STAGE_TOUCH
                       │ key │          on_smtd_action(macro_key, SMTD_ACTION_TOUCH, 0)
                       │     │              
                       │     │              
10ms - - - - - - - - - │ - - │ ┌—————┐ - - - - - - - - - - - - - - - - -
                       │     │ │foll.│  SMTD_STAGE_FOLLOWING_TOUCH            
                       │     │ │ key │  
                       │     │ │     │
20ms - - - - - - - - - └—————┘ │ - - │ - - - - - - - - - - - - - - - - -
                               │     │  SMTD_STAGE_TOUCH_RELEASE
                               │     │
                               │     │
                               │     │
                               │     │
                               │     │
                               │     │
30ms + RELEASE_TERM  - - - - - │ - - │ - - - - - - - - - - - - - - - - -
                               │     │  SMTD_STAGE_NONE
                               │     │  on_smtd_action(macro_key, SMTD_ACTION_TAP, 0)
                               │     │  press(following_key)
                               │     │
2000ms - - - - - - - - - - - - └—————┘ - - - - - - - - - - - - - - - - -
                                        release(following_key)
```
