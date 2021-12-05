//
//  model.h
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef model_h
#define model_h

void model_setup(int numtrains);
void model_set_from_to(int train, int from, int to);

int model_num_states(void);

// state num <-> info
int model_state_num(int posT0, int posT1, int posT2, int posT3);
void model_positions_for_state(int statenum, int *posT0, int *posT1, int *posT2, int *posT3);

int model_initial_state(void);
int model_final_state(void);

typedef enum {
    action_dont_move,
    action_left_straight,
    action_left_turn,
    action_right_straight,
    action_right_turn,
} model_elem_action_t;

/*

int model_num_actions(void);

// action_num <-> actions
int model_action_num(model_elem_action_t T0act, model_elem_action_t T1act, model_elem_action_t T2act, model_elem_action_t T3act);
void model_actions_for_num(int actionnum, model_elem_action_t *T0act, model_elem_action_t *T1act, model_elem_action_t *T2act, model_elem_action_t *T3act);
*/

/* actions are now one train move at a time
   This allows :
   - reduction of number of actions and thus Q matrix
   - easier detection of collision
 */
#define _NUM_ELEM_ACTIONS 5
int model_num_actions(void);
int model_action_num(int train, model_elem_action_t act);
void model_actions_for_num(int actionnum, int *ptrain, model_elem_action_t *pact);



// transitions
int model_new_state(int statenum, int actionnum, double *preward, int *badmove);

// reward
double model_reward(int statenum);


const char *model_descr_action(model_elem_action_t a);

#endif /* model_h */
