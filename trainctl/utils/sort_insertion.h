//
//  sort_insertion.h
//  train_throttle
//
//  Created by Daniel Braun on 06/03/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef sort_insertion_h
#define sort_insertion_h


// sort trigs
// using insersion sort
// https://books.google.fr/books?id=kse_7qbWbjsC&pg=PA116&redir_esc=y#v=onepage&q&f=false


#define SORT_INSERTION(_type, _array, _num, _cmp, _cmpparam) \
  for (int i=1; i<(_num); i++) {                      \
    _type t = (_array)[i];                            \
    int j;                                            \
    for (j=i; j>0 && ((_cmp)((_array)[j-1], t, (_cmpparam))); j--) { \
        (_array)[j] = (_array)[j-1];                  \
    }                                                 \
    (_array)[j] = t;                                  \
}                                                     \


#endif /* sort_insertion_h */
