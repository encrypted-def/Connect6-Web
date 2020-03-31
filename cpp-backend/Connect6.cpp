#include "Connect6.h"

bool DEBUG = false;
MinimaxTree* Tree;
int board[BOARD_SZ][BOARD_SZ];
bool isFirst = true;
int MAX_DEPTH;

void myturn(int x[], int y[], int cnt) {
  Tree->st_time = clock();
  static int isFirst = true;
  if (cnt == 1) { // 내가 첫 수를 둘 때
    isFirst = false;
    Tree->root->depth = 1;
    Tree->root->fullSearched = true;
    POSITION BestPos[MAX_POS_CANDIDATE];
    ull BestScore[MAX_POS_CANDIDATE];
    for (int i = 0; i < MAX_POS_CANDIDATE; i++) {
      BestPos[i] = INVALID_POS;
      BestScore[i] = INVALID_SCORE;
    }
    Tree->GetStartingBestPosition(BLACK, BestPos, BestScore);
    int child_num = 0;
    for (int i = 0; i < MAX_POS_CANDIDATE; i++) {
      if (BestPos[i] == INVALID_POS) break;
      int pScore = (int)(Tree->root->score[0] & 0xffffffffu) + (int)(BestScore[i] >> 32);
      int opScore = (int)(Tree->root->score[0] >> 32) - (int)(BestScore[i] & 0xffffffffu);  // threat은 제거됐을테니 빼준다.
      Node* ch = new Node(pos2move(BestPos[i], INVALID_POS), INVALID_MOVE, BLACK, bw2score(pScore, opScore));
      Tree->root->children.push_back(ch);
      if (child_num == 0) {
        Tree->root->score[1] = bw2score(pScore, opScore);
      }
      child_num++;
      if (child_num == LEAF_NUM) break;
    }   
  }
  if (cnt == 2 and isFirst) {
    isFirst = false;
    Tree->root->depth = 1;
    Tree->root->fullSearched = true;
    MOVES BestMove[LEAF_NUM];
    ull BestScore[LEAF_NUM];
    for (int i = 0; i < LEAF_NUM; i++) {
      BestMove[i] = INVALID_MOVE;
      BestScore[i] = INVALID_SCORE;
    }
    Tree->GetBestMove(BLACK, BestMove, BestScore);
    int child_num = 0;
    for (int i = 0; i < LEAF_NUM; i++) {
      if (BestMove[i] == INVALID_MOVE) break;
      int pScore = (int)(Tree->root->score[0] & 0xffffffffu) + (int)(BestScore[i] >> 32);
      int opScore = (int)(Tree->root->score[0] >> 32) - (int)(BestScore[i] & 0xffffffffu);  // threat은 제거됐을테니 빼준다.
      Node* ch = new Node(BestMove[i], INVALID_MOVE, BLACK, bw2score(pScore, opScore));
      Tree->root->children.push_back(ch);
      if (child_num == 0) {
        Tree->root->score[1] = bw2score(pScore, opScore);
      }
      child_num++;
    }   
  }

  while (true) {
    if (!Tree->expTree()) break;
    if (Tree->root->score[Tree->root->depth] == MY_INF or Tree->root->score[Tree->root->depth] == OP_INF) break;
    if (Tree->root->depth == MAX_DEPTH) break;
  }

  if (Tree->root->children.empty()) { // 자식이 아무도 없을 경우(정상적이라면 생기지 않을 상황인데 중간에 오류가 발생했다거나 INF를 저장해서 update를 미뤘다거나 할 때 생길 수 있음)
    Tree->root->depth = 1;
    Tree->root->fullSearched = true; // 사실 안해줘도 아무 상관 없음
    Tree->expLeaf(Tree->root);
  }

  int search_depth = MAX_DEPTH;
  x[0] = x[1] = y[0] = y[1] = 0x1f;
  
  // 자식에서 필승수를 우선 찾아보자.
  for (auto& ch : Tree->root->children) {
    if (ch->fullSearched) {
      if (search_depth > ch->depth + 1) search_depth = ch->depth + 1;
      if ((!(ch->depth & 1)) and ch->score[ch->depth] == MY_INF) {
        x[0] = move1X(ch->myMove);
        x[1] = move2X(ch->myMove);
        y[0] = move1Y(ch->myMove);
        y[1] = move2Y(ch->myMove);
        break;
      }
      if (((ch->depth & 1)) and ch->score[ch->depth] == OP_INF) {
        x[0] = move1X(ch->myMove);
        x[1] = move2X(ch->myMove);
        y[0] = move1Y(ch->myMove);
        y[1] = move2Y(ch->myMove);
        break;
      }
    }
    else {
      if (search_depth > ch->depth) search_depth = ch->depth;
    }
  }
  // 없을 경우 자식들 중에서 깊이가 가장 얕은 것을 기준으로 최적의 수를 탐색
  if(DEBUG) cout << "search depth " << search_depth << " root score : " << Tree->root->score[search_depth] << endl;
  if (x[0] == 0x1f) {
    for (auto& ch : Tree->root->children) {
      if(DEBUG) cout << move1X(ch->myMove) << ' ' << move1Y(ch->myMove) << ' ' << move2X(ch->myMove) << ' ' << move2Y(ch->myMove) << ' ' << " : " << ch->score[search_depth-1] << '(' << score2cmp(ch->score[search_depth-1]) << ')' << endl;   
      if (Tree->root->score[search_depth] == ch->score[search_depth - 1]) {
        x[0] = move1X(ch->myMove);
        x[1] = move2X(ch->myMove);
        y[0] = move1Y(ch->myMove);
        y[1] = move2Y(ch->myMove);
        if(DEBUG) continue;
        break;
      }
    }
  }
  if (Tree->OOB(x[0], y[0])) { // 절대 도달해서는 안될 위치이지만 어찌됐든 뭔가 엉켜서 이상한 값이 들어가있을 경우
    MOVES tmp = Tree->getArbitraryMoves(BLACK);
    x[0] = move1X(tmp);
    x[1] = move2X(tmp);
    y[0] = move1Y(tmp);
    y[1] = move2Y(tmp);
  }
  if (cnt == 2 and (Tree->OOB(x[1], y[1]) or (x[0]==x[1] and y[0]==y[1]))) { // 절대 도달해서는 안될 위치이지만 어찌됐든 뭔가 엉켜서 이상한 값이 들어가있을 경우
    MOVES tmp = Tree->getArbitraryMoves(BLACK);
    x[0] = move1X(tmp);
    x[1] = move2X(tmp);
    y[0] = move1Y(tmp);
    y[1] = move2Y(tmp);
  } 
  Tree->moveRoot(XY2move(x[0], y[0], x[1], y[1]));
}




int isFree(int x, int y)
{
  return x >= 0 && y >= 0 && x < BOARD_SZ && y < BOARD_SZ && board[x][y] == 0;
}

void init() { 
  isFirst = true;
  for (int i = 0; i < BOARD_SZ; i++)
    for (int j = 0; j < BOARD_SZ; j++)
      board[i][j] = EMPTY;
  Node* _root = new Node(INVALID_MOVE, INVALID_MOVE, WHITE, 0); // 일단 invalid move로 초기화함, root는 opposite로 시작    
  delete Tree->root;
  Tree->root = _root;
  for (int i = 0; i < BOARD_SZ; i++)
    for (int j = 0; j < BOARD_SZ; j++)
      Tree->myBoard[i][j] = EMPTY;
}

bool setting(int level){
  init();
  int timeByLevel[5] = {1, 2, 4, 6, 10};
  int depthByLevel[5] = {1, 3, 6, 8, 10};
  level--;
  if(level < 0 or level > 4) return false;
  Tree->limit = timeByLevel[level];
  MAX_DEPTH = depthByLevel[level];
  return true;
}

bool mymove(int x[], int y[], int cnt) {
  if(isFirst and cnt != 1) return false;
  if(!isFirst and cnt != 2) return false;
  isFirst = false;
  myturn(x, y, cnt);
  for(int i = 0; i < cnt; i++)
    if(!isFree(x[i], y[i])) return false;
  for (int i = 0; i < cnt; i++) {
    if (isFree(x[i], y[i])) {
      board[x[i]][y[i]] = 1;
      Tree->myBoard[x[i]][y[i]] = BLACK;
    }
  }
  return true;
}

// Tree->myBoard 갱신은 moveRoot에서도 이루어짐
bool opmove(int x[], int y[], int cnt) {
  if(isFirst and cnt != 1) return false;
  if(!isFirst and cnt != 2) return false;
  isFirst = false;
  for(int i = 0; i < cnt; i++)
    if(!isFree(x[i], y[i])) return false;
  
  if (cnt == 1) { // 상대가 게임을 시작한 경우, 초기 root의 값, score 변경
    Tree->root->myMove = pos2move(XY2pos(x[0], y[0]), 0x3ff);
    ull score1 = Tree->eval1(XY2pos(x[0], y[0]), WHITE);
    ull myScore = (int)(Tree->root->score[0] >> 32) + (int)(score1 >> 32);
    ull opScore = (int)(Tree->root->score[0] & 0xffffffff) - (int)(score1 & 0xffffffff);
    Tree->root->score[0] = bw2score(myScore, opScore);
  }
  else // 게임 진행 중, 상대 턴이 끝났을 경우, root 자체를 변경
    Tree->moveRoot(pos2move(XY2pos(x[0], y[0]), XY2pos(x[1], y[1])));
  for (int i = 0; i < cnt; i++) {
    board[x[i]][y[i]] = WHITE;
    Tree->myBoard[x[i]][y[i]] = WHITE;
  }
  return true;
}

bool block(int x, int y) {
  if (isFree(x, y)) {
    board[x][y] = BLOCK;
    Tree->myBoard[x][y] = BLOCK;
    return true;
  }
  return false;
}

void showboard(){
  cout << "   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18" << endl;
  for(int i = 0; i < 19; i++){
    if(i<10) cout << 0;
    cout << i << ' ';
    for(int j = 0; j < 19; j++){      
      if(board[i][j]==EMPTY) cout << '.';
      else cout << board[i][j];
      cout << "  ";
    }
    cout << endl;
    if(i != 18)cout << endl;
  }
}
void run(){
  while(true){
    string command;
    cin >> command;
    // SETTING 4
    if(command == "DEBUG"){
      DEBUG = !DEBUG;
      cout << "DEBUG OK" << endl;
  }
    else if(command == "SETTING"){
      int level;
      cin >> level;
      if(setting(level)) cout << "SETTING OK" << endl;
      else cout << "SETTING FAIL" << endl;
    }
    // BLOCK 11 6
    else if(command == "BLOCK"){
      int x, y;
      cin >> x >> y;
      if(block(x, y)) cout << "BLOCK OK" << endl;
      else cout << "BLOCK FAIL" << endl;
      if(DEBUG) showboard();
    }
    // OPMOVE 1 4 6
    else if(command == "OPMOVE"){ // Human Move
      int num;
      cin >> num;
      int x[2], y[2];
      for(int i = 0; i < num; i++) cin >> x[i] >> y[i];
      if(opmove(x, y, num)) cout << "OPMOVE OK" << endl;
      else cout << "OPMOVE FAIL" << endl;
      if(DEBUG) showboard();
    }
    // MYMOVE 2
    else if(command == "MYMOVE"){
      int num;
      cin >> num;
      int x[2], y[2];
      if(!mymove(x, y, num)) cout << "MYMOVE FAIL" << endl;
      else{
        cout << "MYMOVE OK ";
        if(num == 1) cout << num << ' ' << x[0] << ' ' << y[0] << endl;
        else cout << num << ' ' << x[0] << ' ' << y[0] << ' ' << x[1] << ' ' << y[1] << endl;
      }
      if(DEBUG) showboard();
    }
    // QUIT
    else if(command == "QUIT"){
      cout << "QUIT OK" << endl;
      break;
    }
    else{
      cout << "Unrecognized command " << command << endl;
    }
  }
}

void test(){
  // unsigned check
  assert(score2cmp(18442469876872387361) == 3516401);
}
int main() {
  Tree = new MinimaxTree();
  run();  
}







