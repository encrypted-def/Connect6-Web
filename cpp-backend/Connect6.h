#include <iostream>
#include <assert.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <set>
#include <assert.h>
#include <queue>
#include <stack>
using namespace std;

typedef unsigned int POSITION;
typedef unsigned int MOVES;
typedef unsigned int uint;
typedef unsigned long long ull;
// limitPos엔 최대 3개까지만 담을 것임

#define EMPTY 0
#define BLACK 1 // 나는 black
#define WHITE 2 // 상대는 white
#define BLOCK 3
#define MY_INF 0x004c4b4000000000
#define OP_INF 0x00000000004c4b40
#define INF 5000000
#define posX(p) (((p) >> 5) & 0x1f)
#define posY(p) ((p) & 0x1f)
#define move1X(m) (((m) >> 15) & 0x1f)
#define move1Y(m) (((m) >> 10) & 0x1f)
#define move2X(m) (((m) >> 5) & 0x1f)
#define move2Y(m) ((m) & 0x1f)
#define move1pos(m) (((m) >> 10) & 0x3ff)
#define move2pos(m) ((m) & 0x3ff)
#define XY2pos(x,y) (((x)<<5) | (y))
#define XY2move(x1,y1,x2,y2) (((x1)<<15) |((y1)<<10) |((x2)<<5) | (y2))
#define pos2move(p1,p2) (((p1)<<10) | (p2))
#define bw2score(b,w) ((((ull)(b))<<32) ^ (w))
#define score2cmp_delta(S) (int) ((int)((S) >> 32) + SCORE_FACTOR * (int)((S) & 0xffffffff)) // my_score가 내 점수이고 op_score가 상대를 block한 점수일 때
#define score2cmp(S) (int) ((int)((S) >> 32) - SCORE_FACTOR * (int)((S) & 0xffffffff)) // my_score가 내 점수이고 op_score가 상대 점수일 때


// my score, opponent score로 주어져야함에 유의

const int LEAF_NUM = 7;
const int threatBonus = 1000000;
const POSITION INVALID_POS = 0x3ff;
const MOVES INVALID_MOVE = 0xfffff;
const ull INVALID_SCORE = 0x3f3f3f3f3f3f3f3f;
extern int MAX_DEPTH;
extern bool DEBUG;
const int MAX_POS_CANDIDATE = 30;
const double SCORE_FACTOR = 1.5;
const int BOARD_SZ = 19;
class Node {
public:
  int depth; // 현재 이후로 몇 수 앞까지 확인했는가?
  int player; // 1 : 나, 2 : 상대
  bool fullSearched; // depth까지 전부 search가 완료되었는가?
  ull score[12]; // score[i] : i단계 들어갔을 때 최대 score
                            // player 관계 없이 현재 node의 player의 점수는 상위 32비트, opponent의 점수는 하위 32비트
                            // 0x3f3f3f3f3f3f3f3f가 INVALID임을 의미
                            // N단계 이후의 player 관점에서 점수를 대입시킴
  MOVES opMove, myMove; // 직전 상대의 move, 이번 나의 move. pair에서 int로 전환할 경우 초기화시켜야 함
  vector<Node*> children;
  Node(MOVES _myMove, MOVES _opMove, int _player, ull _score);
  ~Node();
};
class MinimaxTree {
public:
  Node * root;
  int myBoard[BOARD_SZ][BOARD_SZ];
  int limit;
  clock_t st_time;
  MinimaxTree();
  void setMove(MOVES mv, int state);
  void setPosition(POSITION pt, int state);
  void moveRoot(MOVES _opMove);
  bool expFullTree(Node* cur);
  bool expTree();
  bool expLeaf(Node* leaf); // leaf에서 한 단게 확장
  bool incDepth(Node* cur); // depth 1단계 증가시킴
  bool OOB(int x, int y); // (x,y)가 board 위에 있는지 판단
  int threatCnt(MOVES opMoves, int player);
  ull eval1(POSITION pt, int player);
  ull eval2(MOVES myMove, int player);
  ull modified_eval1(POSITION pt, int player);
  MOVES Con6Move(MOVES myMove, int player);
  bool IsNoThreat(MOVES myMove, int player);
  POSITION Con6Begin1();
  MOVES Con6Begin2();
  int GetForcedMove(MOVES opMoves, int player, MOVES ForcedMove[]);
  void GetSingleBestPosition(int player, POSITION BestPosition[], ull BestScore[]);
  void GetStartingBestPosition(int player, POSITION BestPosition[], ull BestScore[]);
  void GetBestMove(int player, MOVES BestMove[], ull BestScore[]);
  MOVES getArbitraryMoves(int player);

};

extern MinimaxTree* Tree;