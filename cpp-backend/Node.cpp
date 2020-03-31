#include "Connect6.h"

Node::Node(MOVES _myMove, MOVES _opMove, int _player, ull _score) {
  myMove = _myMove;
  opMove = _opMove;
  depth = 0;
  player = _player;
  fullSearched = true;
  score[0] = _score;
  for (int i = 1; i <= 11; i++)
    score[i] = INVALID_SCORE;
}
Node::~Node() { // root를 지울 때는 자식 정보를 다 날린 이후에 지워야 함
  for (auto& child : children)
    delete child;
}