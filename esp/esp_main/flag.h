#ifndef FLAG_H
#define FLAG_H

// mailbox pour communication inter-tâche

enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  int val;
};

#endif