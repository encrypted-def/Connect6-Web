#include "Connect6.h"

MinimaxTree::MinimaxTree() {
  Node* _root = new Node(INVALID_MOVE, INVALID_MOVE, WHITE, 0); // 일단 invalid move로 초기화함, root는 opposite로 시작
  root = _root;
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++)
      myBoard[i][j] = EMPTY;
}

// valid한 빈 Moves를 반환함
MOVES MinimaxTree::getArbitraryMoves(int player) {
  POSITION tmp1 = INVALID_POS;
  for (int i = 0; i < 19; i++) {
    for (int j = 0; j < 19; j++) {
      if (myBoard[i][j] == EMPTY) {
        if (tmp1 == INVALID_POS)
          tmp1 = XY2pos(i, j);
        else {
          return pos2move(tmp1, XY2pos(i, j));
        }
      }
    }
  }
  return INVALID_MOVE;
}
void MinimaxTree::setPosition(POSITION pt, int state) {
  myBoard[posX(pt)][posY(pt)] = state;
}

bool MinimaxTree::OOB(int x, int y) {
  return (0 > x || x >= BOARD_SZ || 0 > y || y >= BOARD_SZ);
}

// 6목을 만드는 착수를 반환. 없으면 INVALID_MOVE
MOVES MinimaxTree::Con6Move(MOVES myMove, int player) {
  int dx[4] = { -1, 0, 1, 1 };
  int dy[4] = { 1, 1, 1, 0 };
  for (int iter = 0; iter < 2; iter++) {
    for (int dir = 0; dir < 4; dir++) {
      int st_x, st_y;
      if (iter == 0) {
        if (move1X(myMove) == 0x1f) continue;
        st_x = move1X(myMove) - 6 * dx[dir];
        st_y = move1Y(myMove) - 6 * dy[dir];
      }
      else {
        if (move2X(myMove) == 0x1f) continue;
        st_x = move2X(myMove) - 6 * dx[dir];
        st_y = move2Y(myMove) - 6 * dy[dir];
      }
      int opponent = 3 ^ player;
      int pStone = 0;
      int opStone = 0;
      int blockStone = 0;
      bool init = false;
      for (int i = 0; i < 6; i++) { // 초기 6개의 돌
        st_x += dx[dir];
        st_y += dy[dir];
        if (OOB(st_x, st_y) || OOB(st_x + 5 * dx[dir], st_y + 5 * dy[dir])) continue;
        if (!init) {
          init = true;
          for (int j = 0; j < 6; j++) {
            if (myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == player)
              pStone++;
            else if (myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == BLOCK)
              blockStone++;
            else if(myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == opponent)
              opStone++;
          }
        }
        else {
          if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == player)
            pStone--;
          else if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == BLOCK)
            blockStone--;
          else if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == opponent)
            opStone--;

          if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == player)
            pStone++;
          else if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == BLOCK)
            blockStone++;
          else if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == opponent)
            opStone++;
        }
        // 내 돌이 4개 미만 or 적 돌이 있음 or block이 있음
        if (pStone < 4 || opStone != 0 || blockStone != 0)
          continue;
        if (pStone == 4) {
          MOVES ret = INVALID_MOVE;
          for (int j = 0; j < 6; j++) {
            if (myBoard[st_x][st_y] == EMPTY) {
              if (move1X(ret) == 0x1f) ret = ret ^ 0xffc00 ^ (XY2pos(st_x, st_y) << 10);
              else {
                ret = ret ^ 0x3ff ^ XY2pos(st_x, st_y);
                return ret;
              }
            }
            st_x += dx[dir];
            st_y += dy[dir];
          }
        }
        if (pStone == 5) {
          MOVES ret = INVALID_MOVE;
          for (int j = 0; j < 6; j++) {
            if (myBoard[st_x][st_y] == EMPTY) {
              if (move1X(ret) == 0x1f) {
                ret = (ret ^ 0xffc00) | (XY2pos(st_x, st_y) << 10);
                break;
              }
            }
            st_x += dx[dir];
            st_y += dy[dir];
          } // st_x, st_y 만으로 6목 완성. 이제 적절한 빈자리만 끼워넣으면 끝
          for (int i = 0; i < BOARD_SZ; i++) {
            for (int j = 0; j < BOARD_SZ; j++) {
              if (myBoard[i][j] == EMPTY) { 
                return (ret ^ 0x3ff) | XY2pos(i, j);
                break;
              }
            }
            if (move2X(ret) != 0x1f) break;
          }
          if (move2X(ret) != 0x1f) return ret; // 5개의 돌이 연속해있고, 다른 돌을 둘 빈 곳을 찾은 경우
        }
      }
    }
  }
  return INVALID_MOVE;
}
// Threat이 사라졌는가?(즉 player가 6목을 만들 방법이 없어졌는가?)
// myMove에는 살펴봐야하는 지점.(Threat의 남은 칸)
bool MinimaxTree::IsNoThreat(MOVES myMove, int player) {
  int dx[4] = { -1, 0, 1, 1 };
  int dy[4] = { 1, 1, 1, 0 };
  for (int iter = 0; iter < 2; iter++) {
    for (int dir = 0; dir < 4; dir++) {
      int st_x, st_y;
      if (iter == 0) {
        if (move1X(myMove) == 0x1f) continue;
        st_x = move1X(myMove) - 6 * dx[dir];
        st_y = move1Y(myMove) - 6 * dy[dir];
      }
      else {
        if (move2X(myMove) == 0x1f) continue;
        st_x = move2X(myMove) - 6 * dx[dir];
        st_y = move2Y(myMove) - 6 * dy[dir];
      }
      int opponent = 3 ^ player;
      int pStone = 0;
      int opStone = 0;
      int blockStone = 0;
      bool init = false;
      for (int i = 0; i < 6; i++) { // 초기 6개의 돌
        st_x += dx[dir];
        st_y += dy[dir];
        if (OOB(st_x, st_y) || OOB(st_x + 5 * dx[dir], st_y + 5 * dy[dir])) continue;
        if (!init) {
          init = true;
          for (int j = 0; j < 6; j++) {
            if (myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == player)
              pStone++;
            else if (myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == BLOCK)
              blockStone++;
            else if (myBoard[st_x + j * dx[dir]][st_y + j * dy[dir]] == opponent)
              opStone++;
          }
        }
        else {
          if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == player)
            pStone--;
          else if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == BLOCK)
            blockStone--;
          else if (myBoard[st_x - dx[dir]][st_y - dy[dir]] == opponent)
            opStone--;

          if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == player)
            pStone++;
          else if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == BLOCK)
            blockStone++;
          else if (myBoard[st_x + 5 * dx[dir]][st_y + 5 * dy[dir]] == opponent)
            opStone++;
        }
        // 내 돌이 4개 미만 or 적 돌이 있음 or block이 있음
        if (pStone < 4 || opStone != 0 || blockStone != 0)
          continue;
        // 정확하게 하려면 5개의 pStone이 있을 경우에는 바로 return하는 것이 아니라, 다른 한 돌까지 놓을 자리를 찾아야하나, 현실적으로 극후반까지 가지는 않을 것이니 계산량을 줄이자.
        return false;
      }
    }
  }
  return true; // 잘 막혔으니 여기에 도달함
}
void MinimaxTree::moveRoot(MOVES _opMove) { // 현재 root을 value로 옮김. value를 착수한 것으로 생각해도 무방.
  Node* newRoot = nullptr;
  for (auto& child : root->children) { // 일단 모든 현재 root의 자식들에 대해
    if (child->myMove == _opMove)
      newRoot = child;
    else // 버려질 자식이면 전부 제거
      delete child;
  }
  root->children.clear();

  if (!newRoot) { // 현재 나의 후보군중에 존재하지 않는 착수라면
    ull nextScore = eval2(_opMove, WHITE);
    int blackScore = (int)(root->score[0] >> 32) - (int)(nextScore & 0xffffffff);
    int whiteScore = (int)(nextScore >> 32) + (int)(root->score[0] & 0xffffffff);
    // black의 threat은 모두 없어졌을 것이라고 추정하고 score에서 threat만큼은 없앰. 만약 그렇지 않다면 알아서 다음 턴에 이겨주겠지
    while (blackScore >= 0.8*threatBonus)
      blackScore -= threatBonus;
    myBoard[move1X(_opMove)][move1Y(_opMove)] = WHITE;
    myBoard[move2X(_opMove)][move2Y(_opMove)] = WHITE; // 착수 시켜두고 threatCnt 계산
    newRoot = new Node(_opMove, root->myMove, WHITE, bw2score(whiteScore, blackScore));
    int dx[4] = { 1,0,1,1 };
    int dy[4] = { 0,1,1,-1 };
    for (int iter = 0; iter < 2; iter++) {
      for (int dir = 0; dir < 4; dir++) {
        int curX, curY;
        if (iter == 0) {
          curX = move1X(_opMove) - 6 * dx[dir];
          curY = move1Y(_opMove) - 6 * dy[dir];
        }
        else {
          curX = move2X(_opMove) - 6 * dx[dir];
          curY = move2Y(_opMove) - 6 * dy[dir];
        }
      }
    }
  }
  delete root; // 현재 root 또한 제거 
  root = newRoot; // root 이동  
}

// 최대 N개의 BestPosition을 반환할 것이다.
// BestPosition에는 INVALID_SCORE가 담겨서 넘어와야 함
void MinimaxTree::GetStartingBestPosition(int player, POSITION BestPosition[], ull BestScore[]) {
  int len = 0;
  int poslist[BOARD_SZ * BOARD_SZ];
  for(int i = 0; i < BOARD_SZ*BOARD_SZ; i++) poslist[i] = i;
  sort(poslist, poslist+BOARD_SZ*BOARD_SZ,
    [](int i1, int i2) -> bool
    {
      return abs(posX(i1)-(BOARD_SZ/2))+abs(posY(i1)-(BOARD_SZ/2)) < abs(posX(i2)-(BOARD_SZ/2))+abs(posY(i2)-(BOARD_SZ/2));
    }
  ); // 중간에 가까운 것 먼저 보게 리스트를 설정
  for (int i = 0; i < BOARD_SZ * BOARD_SZ; i++) {
    int x = posX(poslist[i]);
    int y = posY(poslist[i]);
    if (myBoard[x][y] != EMPTY) continue;
    ull val = eval1(poslist[i], player);
    if (len == 0) {
      BestScore[len] = val;
      BestPosition[len++] = poslist[i];
      continue;
    }
    int convert_score = score2cmp_delta(val);
    if (convert_score <= score2cmp_delta(BestScore[len - 1])) {
      if (len == MAX_POS_CANDIDATE) continue;
      BestScore[len] = val;
      BestPosition[len++] = poslist[i];
      continue;
    }
    int idx = len - 1;
    while (idx >= 0 and convert_score > score2cmp_delta(BestScore[idx])) {
      if (idx != MAX_POS_CANDIDATE - 1) {
        BestScore[idx + 1] = BestScore[idx];
        BestPosition[idx + 1] = BestPosition[idx];
      }
      idx--;
    }
    BestScore[idx + 1] = val;
    BestPosition[idx + 1] = poslist[i];
  }
}




// 최대 N개의 BestPosition을 반환할 것이다.
// BestPosition에는 INVALID_SCORE가 담겨서 넘어와야 함
void MinimaxTree::GetSingleBestPosition(int player, POSITION BestPosition[], ull BestScore[]) {
  int len = 0;
  for (int i = 0; i < BOARD_SZ; i++) {
    for (int j = 0; j < BOARD_SZ; j++) {
      if (myBoard[i][j] != EMPTY) continue;
      ull val = eval1(XY2pos(i, j), player);
      if (len == 0) {
        BestScore[len] = val;
        BestPosition[len++] = XY2pos(i, j);
        continue;
      }
      int convert_score = score2cmp_delta(val);
      if (convert_score <= score2cmp_delta(BestScore[len - 1])) {
        if (len == MAX_POS_CANDIDATE) continue;
        BestScore[len] = val;
        BestPosition[len++] = XY2pos(i, j);
        continue;
      }
      int idx = len - 1;
      while (idx >= 0 and convert_score > score2cmp_delta(BestScore[idx])) {
        if (idx != MAX_POS_CANDIDATE - 1) {
          BestScore[idx + 1] = BestScore[idx];
          BestPosition[idx + 1] = BestPosition[idx];
        }
        idx--;
      }
      BestScore[idx + 1] = val;
      BestPosition[idx + 1] = XY2pos(i, j);
    }
  }
}

// player가 pt에 착수했을 때 변화하는 board의 점수
// 상대의 공격 포인트를 막았을 때 / 내 공격 포인트를 확보했을 때 점수 추가
// 증가될 나의 점수, 감소될 상대의 점수를 ull로 묶어서 반환
// ex : 000000010 00000005 이면 내 점수 10점 증가, 상대 점수 5점 감소
ull MinimaxTree::eval1(POSITION pt, int player) {
  bool markBoard[BOARD_SZ][BOARD_SZ] = { 0, };
  int factor[6] = { 0, 10, 396, 1205, 0, 0 }; // 4, 5개는 어차피 Threat에서 count 될 것임
  int opponent = 3 ^ player;
  int myScore = 0;
  int oppoScore = 0;
  
  int dx[4] = { 1,0,1,1 };
  int dy[4] = { 0,1,1,-1 };
  for(int dir = 0; dir < 4; dir++){
    bool init = false;
    int pStone = 0;
    int opStone = 0;
    int blockStone = 0;
    int markStone = 0; // empty와 똑같으나 이게 1 이상이면 Threat으로 쳐지지는 않음
    int curX = posX(pt) - 11 * dx[dir];
    int curY = posY(pt) - 11 * dy[dir];
    for(int i = 0; i < 23; i++){
      curX += dx[dir]; curY += dy[dir];
      if (OOB(curX, curY) || OOB(curX + 5 * dx[dir], curY + 5 * dy[dir])) continue;
      if (!init) {
        init = true;
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        for (int j = 0; j < 6; j++) {
          nX += dx[dir];
          nY += dy[dir];
          // empty이긴한데 mark가 놓여있을 경우
          if (markBoard[nX][nY])
            markStone++;
          else if (myBoard[nX][nY] == player)
            pStone++;
          else if (myBoard[nX][nY] == opponent)
            opStone++;
          else if (myBoard[nX][nY] == BLOCK)
            blockStone++;
        }
      }
      else {
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        if (markBoard[nX][nY])
          markStone--;
        else if (myBoard[nX][nY] == player)
          pStone--;
        else if (myBoard[nX][nY] == opponent)
          opStone--;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone--;
        nX = curX + 5 * dx[dir];
        nY = curY + 5 * dy[dir];
        if (markBoard[nX][nY])
          markStone++;        
        else if (myBoard[nX][nY] == player)
          pStone++;
        else if (myBoard[nX][nY] == opponent)
          opStone++;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone++;
      }
      if(pStone >= 4 && markStone == 0 && opStone == 0 && blockStone == 0){
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        for (int i = 0; i < 6; i++) {
          nX += dx[dir];
          nY += dy[dir];
          // 빈칸이고 해당칸에 mark가 놓여있지 않다면
          if (myBoard[nX][nY] == EMPTY && !markBoard[nX][nY]) {
            markBoard[nX][nY] = true;
            markStone++;
          }
        }
      }
    }
  }
  for (int dir = 0; dir < 4; dir++) {
    bool init = false;
    int pStone = 0;
    int opStone = 0;
    int blockStone = 0;
    int markStone = 0; // empty와 똑같으나 이게 1 이상이면 Threat으로 쳐지지는 않음
    myBoard[posX(pt)][posY(pt)] = player; // Threat의 수를 구하는 것 때문에 일단 착수시켜 둠
    int curX = posX(pt) - 6 * dx[dir];
    int curY = posY(pt) - 6 * dy[dir];
    for (int i = 0; i < 6; i++) {
      curX += dx[dir];
      curY += dy[dir];
      if (OOB(curX, curY) || OOB(curX + 5 * dx[dir], curY + 5 * dy[dir])) continue;
      if (!init) {
        init = true;
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        for (int j = 0; j < 6; j++) {
          nX += dx[dir];
          nY += dy[dir];
          // empty이긴한데 mark가 놓여있을 경우
          if (markBoard[nX][nY])
            markStone++;
          else if (myBoard[nX][nY] == player)
            pStone++;
          else if (myBoard[nX][nY] == opponent)
            opStone++;
          else if (myBoard[nX][nY] == BLOCK)
            blockStone++;
        }
      }
      else {
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        if (markBoard[nX][nY])
          markStone--;
        else if (myBoard[nX][nY] == player)
          pStone--;
        else if (myBoard[nX][nY] == opponent)
          opStone--;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone--;
        nX = curX + 5 * dx[dir];
        nY = curY + 5 * dy[dir];
        if (markBoard[nX][nY])
          markStone++;        
        else if (myBoard[nX][nY] == player)
          pStone++;
        else if (myBoard[nX][nY] == opponent)
          opStone++;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone++;
      }
      // 1. 이 수로 인해 새롭게 생겨난 나의 공격포인트. 해당 window에 상대 돌과 BLOCK이 없어야 함
      if (opStone == 0 && blockStone == 0) {
        if (pStone == 4 && markStone == 0) { // 딱 4개가 채워진 경우만 Threat. 그렇지 않다면 이전에 이미 Threat이었다는 의미니 고려할 필요 없음.
          myScore += threatBonus;
          int nX = curX - dx[dir];
          int nY = curY - dy[dir];
          for (int i = 0; i < 6; i++) {
            nX += dx[dir];
            nY += dy[dir];
            // 빈칸이고 해당칸에 mark가 놓여있지 않다면
            if (myBoard[nX][nY] == EMPTY && !markBoard[nX][nY]) {
              markBoard[nX][nY] = true;
              markStone++;
            }
          }
        }
        myScore += (factor[pStone] - factor[pStone - 1]);
      }
      // 2. 이 수로 인해 막은 상대의 공격포인트. 양 끝이 막혀있고 상대 금수가 없어야 함
      if (pStone == 1 && blockStone == 0)
        oppoScore += factor[opStone]; // 상대턴에 빼졌을테니 막고나면 더해줌0
    }
  }
  myBoard[posX(pt)][posY(pt)] = EMPTY; // 보드 되돌림
  if (myScore < 0) myScore = 0; // 공격 자리를 없애버리기만 해서 음수로 갔을 경우엔 0으로 만들어버림
  return ((ull)myScore << 32) | oppoScore;
}
// Threat이 없을 때 N개의 BestMove를 구해줌. 이 때 limitPos과는 별도로 ValidMove인지 확인해야함
// BestMove에는 INVALID_MOVE, INVALID_SCORE가 찬 상태로 들어옴
void MinimaxTree::GetBestMove(int player, MOVES BestMove[], ull BestScore[]) {
  POSITION CandPos[MAX_POS_CANDIDATE];
  ull CandScore[MAX_POS_CANDIDATE];
  for (int i = 0; i < MAX_POS_CANDIDATE; i++) {
    CandScore[i] = INVALID_SCORE;
    CandPos[i] = INVALID_POS;
  }
  GetSingleBestPosition(player, CandPos, CandScore);
  int pos_sz = 1;
  while (pos_sz < MAX_POS_CANDIDATE and CandPos[pos_sz - 1] != INVALID_POS)
    pos_sz++;
  int len = 0;
  for (int i = 0; i < pos_sz / 2; i++) {
    POSITION tmp1 = CandPos[i];
    ull score1 = CandScore[i];
    setPosition(tmp1, player); // 일단 착수를 시켜둠
    for (int j = i + 1; j < pos_sz; j++) {
      POSITION tmp2 = CandPos[j];
      ull score2 = 0;
      int x1 = posX(tmp1);
      int x2 = posX(tmp2);
      int y1 = posY(tmp1);
      int y2 = posY(tmp2);

      // 서로 영향을 줄 수 없는 Position이라면 굳이 함수를 한 번 더 부르며 시간낭비할 필요가 없다.
      if (x1 != x2 && y1 != y2 && (x1 - y1) != (x2 - y2) && (x1 + y1) != (x2 + y2))
        score2 = CandScore[j];
      else
        score2 = eval1(tmp2, player);

      ull tot_score = score1 + score2;
      if (len == 0) {
        BestScore[len] = tot_score;
        BestMove[len++] = pos2move(tmp1, tmp2);
        continue;
      }
      int convert_score = score2cmp_delta(tot_score);
      if (convert_score <= score2cmp_delta(BestScore[len - 1])) {
        if (len == LEAF_NUM) continue;
        BestScore[len] = tot_score;
        BestMove[len++] = pos2move(tmp1, tmp2);
        continue;
      }
      int idx = len - 1;
      while (idx >= 0 and convert_score > score2cmp_delta(BestScore[idx])) {
        if (idx != LEAF_NUM - 1) {
          BestScore[idx + 1] = BestScore[idx];
          BestMove[idx + 1] = BestMove[idx];
        }
        idx--;
      }
      BestScore[idx + 1] = tot_score;
      BestMove[idx + 1] = pos2move(tmp1, tmp2);
    }
    setPosition(tmp1, EMPTY);
  }
}

// player가 pt에 착수했을 때 변화하는 board의 점수

// 상대의 공격 포인트를 막았을 때 / 내 공격 포인트를 확보했을 때 점수 추가
// 증가될 나의 점수, 감소될 상대의 점수를 ull로 묶어서 반환
// ex : 000000010 00000005 이면 내 점수 10점 증가, 상대 점수 5점 감소
// threat은 건드리지 않음.
ull MinimaxTree::modified_eval1(POSITION pt, int player) {
  int factor[6] = { 0, 10, 396, 1205, 0, 0 };
  int opponent = 3 ^ player;
  int myScore = 0;
  int oppoScore = 0;
  myBoard[posX(pt)][posY(pt)] = player; // Threat의 수를 구하는 것 때문에 일단 착수시켜 둠
  int dx[4] = { 1,0,1,1 };
  int dy[4] = { 0,1,1,-1 };
  for (int dir = 0; dir < 4; dir++) {
    bool init = false;
    int pStone = 0;
    int opStone = 0;
    int blockStone = 0;
    int curX = posX(pt) - 5 * dx[dir];
    int curY = posY(pt) - 5 * dy[dir];
    for (int i = 0; i < 6; i++) {
      curX += dx[dir];
      curY += dy[dir];
      if (OOB(curX, curY) || OOB(curX + 5 * dx[dir], curY + 5 * dy[dir])) continue;
      if (!init) {
        init = true;
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        for (int j = 0; j < 6; j++) {
          nX += dx[dir];
          nY += dy[dir];
          if (myBoard[nX][nY] == player)
            pStone++;
          else if (myBoard[nX][nY] == opponent)
            opStone++;
          else if (myBoard[nX][nY] == BLOCK)
            blockStone++;
        }
      }
      else {
        int nX = curX - dx[dir];
        int nY = curY - dy[dir];
        if (myBoard[nX][nY] == player)
          pStone--;
        else if (myBoard[nX][nY] == opponent)
          opStone--;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone--;
        nX = curX + 5 * dx[dir];
        nY = curY + 5 * dy[dir];
        if (myBoard[nX][nY] == player)
          pStone++;
        else if (myBoard[nX][nY] == opponent)
          opStone++;
        else if (myBoard[nX][nY] == BLOCK)
          blockStone++;
      }
      // 1. 이 수로 인해 새롭게 생겨난 나의 공격포인트. 해당 window에 상대 돌, BLOCK가 없어야 함
      if (opStone == 0 && blockStone == 0) {
        myScore += (factor[pStone] - factor[pStone - 1]);
      }
      // 2. 이 수로 인해 막은 상대의 공격포인트. 
      if (pStone == 1 && blockStone == 0)
        oppoScore += factor[opStone]; // 상대턴에 빼졌을테니 막고나면 더해줌
    
    }
  }
  myBoard[posX(pt)][posY(pt)] = EMPTY; // 보드 되돌림
  if (myScore < 0) myScore = 0; // 공격 자리를 없애버리기만 해서 음수로 갔을 경우엔 0으로 만들어버림
  return ((ull)myScore << 32) | oppoScore;
}

ull MinimaxTree::eval2(MOVES myMove, int player) {
  ull score = 0;
  // 우선 threatCnt를 제외한 점수
  score += modified_eval1(move1pos(myMove), player);
  myBoard[move1X(myMove)][move1Y(myMove)] = player;
  score += modified_eval1(move2pos(myMove), player);
  myBoard[move2X(myMove)][move2Y(myMove)] = player;
  score += (ull)((threatCnt(myMove, player)*threatBonus)) << 32;
  myBoard[move1X(myMove)][move1Y(myMove)] = EMPTY;
  myBoard[move2X(myMove)][move2Y(myMove)] = EMPTY;
  return score;
}

void MinimaxTree::setMove(MOVES mv, int state) {
  if (move1X(mv) != 0x1f)
    myBoard[move1X(mv)][move1Y(mv)] = state;
  if (move2X(mv) != 0x1f)
    myBoard[move2X(mv)][move2Y(mv)] = state;
}
// 나의 착수로 인해 생긴 Threat의 갯수는 무엇인가?
// map에 opMoves가 착수된 상태로 호출되어야 함
int MinimaxTree::threatCnt(MOVES myMoves, int player) {
  int tCnt = 0;
  bool markBoard[BOARD_SZ][BOARD_SZ] = { 0, };
  // 아직 상대가 착수하지 않았다면
  if (move1X(myMoves) == 0x1f)
    return 0;
  int opponent = 3 ^ player;
  int dx[4] = { -1,0,1,1 };
  int dy[4] = { 1,1,1,0 };
  for (int iter = 0; iter < 2; iter++) {
    int st_x, st_y;
    if (iter == 0) {
      st_x = move1X(myMoves);
      st_y = move1Y(myMoves);
    }
    else {
      st_x = move2X(myMoves);
      st_y = move2Y(myMoves);
    }
    if (st_x == 0x1f) continue;
    for (int dir = 0; dir < 4; dir++) {
      int curX = st_x - 6 * dx[dir];
      int curY = st_y - 6 * dy[dir]; // i=0일 때 바로 curX, curY에 dx[dir],dy[dir]을 더하므로 6을 곱함
      bool init = false;
      int pStone = 0; // 해당 window 내에 player의 돌의 수 
      int opStone = 0; // opponent의 돌의 수
      int blockStone = 0; // BLOCK 수
      int markStone = 0;
      for (int i = 0; i < 6; i++) {
        curX += dx[dir];
        curY += dy[dir];
        if (OOB(curX, curY) || OOB(curX + 5 * dx[dir], curY + 5 * dy[dir]))
          continue;
        if (!init) {
          init = true;
          for (int j = 0; j < 6; j++) {
            int nX = curX + j * dx[dir];
            int nY = curY + j * dy[dir];
            if (markBoard[nX][nY])
              markStone++;
            else if (myBoard[nX][nY] == opponent)
              opStone++;
            else if (myBoard[nX][nY] == player)
              pStone++;
            else if (myBoard[nX][nY] == BLOCK)
              blockStone++;
          }
        }
        else {
          if (markBoard[curX - dx[dir]][curY - dy[dir]])
            markStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == opponent)
            opStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == player)
            pStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == BLOCK)
            blockStone--;

          if (markBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]])
            markStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == opponent)
            opStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == player)
            pStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == BLOCK)
            blockStone++;
        }

        if (markStone == 0 && blockStone == 0 && opStone == 0 && pStone >= 4) {
          tCnt++;
          int tmpX = curX - dx[dir];
          int tmpY = curY - dy[dir];
          for (int j = 0; j < 6; j++) {
            tmpX += dx[dir];
            tmpY += dy[dir];
            if(myBoard[tmpX][tmpY] == EMPTY && !markBoard[tmpX][tmpY]){
              markBoard[tmpX][tmpY] = true;
              markStone++;
            }
          }
        }
      }
    }
  } // ThreatList 탐색 완료
  return tCnt;
}
// 상대의 승리를 막기 위해 반드시 두어야하는 자리들의 목록을 ForcedMove에 넣음.
// (만약 Threat의 수가 1일 경우 {{x, y}, INVALID_POS}을 반환. tCnt에는 threat의 수를 기록할 것임
// board에 opponent의 6목은 없다고 가정.(있을 경우 밑의 tmp 부분에서 오류 발생함)
// tCnt 갯수 반환
// ForcedMove에는 INVALID_MOVE를 4개 채워서 이 함수를 call해야 함
int MinimaxTree::GetForcedMove(MOVES opMoves, int player, MOVES ForcedMove[]) {
  unsigned long long ret = 0;
  int tCnt = 0;
  int fidx = 0;
  // 아직 상대가 착수하지 않았다면
  if (move1X(opMoves) == 0x1f) return 0;
  int opponent = 3 ^ player;
  int dx[4] = { -1,0,1,1 };
  int dy[4] = { 1,1,1,0 };
  MOVES ThreatList[3] = { INVALID_MOVE,INVALID_MOVE,INVALID_MOVE };
  bool markBoard[BOARD_SZ][BOARD_SZ] = { 0, };
  for (int iter = 0; iter < 2; iter++) {
    int st_x, st_y;
    if (iter == 0) {
      st_x = move1X(opMoves);
      st_y = move1Y(opMoves);
    }
    else {
      st_x = move2X(opMoves);
      st_y = move2Y(opMoves);
    }
    if (st_x == 0x1f) continue;
    for (int dir = 0; dir < 4; dir++) {
      int curX = st_x - 6 * dx[dir];
      int curY = st_y - 6 * dy[dir];
      bool init = false;
      int pStone = 0; // 해당 window 내에 player의 돌의 수 
      int opStone = 0; // opponent의 돌의 수
      int blockStone = 0;
      int markStone = 0;
      for (int i = 0; i < 6; i++) {
        // curX ~ curX+5*dx[dir] window
        curX += dx[dir];
        curY += dy[dir];
        if (OOB(curX, curY) || OOB(curX + 5 * dx[dir], curY + 5 * dy[dir]))
          continue;

        if (!init) {
          init = true;
          for (int j = 0; j < 6; j++) {
            int nX = curX + j * dx[dir];
            int nY = curY + j * dy[dir];
            if (markBoard[nX][nY])
              markStone++;
            else if (myBoard[nX][nY] == opponent)
              opStone++;
            else if (myBoard[nX][nY] == player)
              pStone++;
            else if (myBoard[nX][nY] == BLOCK)
              blockStone++;
          }
        }
        else {
          if (markBoard[curX - dx[dir]][curY - dy[dir]])
            markStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == opponent)
            opStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == player)
            pStone--;
          else if (myBoard[curX - dx[dir]][curY - dy[dir]] == BLOCK) // BLOCK도 상대 턴에서는 상대가 활용할 수 있으니 상대 돌이라고 간주
            blockStone--;

          if (markBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]])
            markStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == opponent)
            opStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == player)
            pStone++;
          else if (myBoard[curX + 5 * dx[dir]][curY + 5 * dy[dir]] == BLOCK) // BLOCK도 상대 턴에서는 상대가 활용할 수 있으니 상대 돌이라고 간주
            blockStone++;
        }
        if (markStone == 0 && blockStone == 0 && pStone == 0 && opStone >= 4) {
          tCnt++;
          if (tCnt > 2) { // 어차피 막아야 할 곳이 3곳 이상이라 졌음. 시간 아까우니 그냥 return
            ForcedMove[0] = (pos2move(move2pos(ThreatList[0]), move2pos(ThreatList[1])));
            return 3;
          }
          MOVES tmp = INVALID_MOVE; // ThreatList에 넣을 Threat
          int tmpX = curX - dx[dir];
          int tmpY = curY - dy[dir];
          for (int j = 0; j < 6; j++) {
            tmpX += dx[dir];
            tmpY += dy[dir];
            if (myBoard[tmpX][tmpY] == EMPTY && !markBoard[tmpX][tmpY]) {
              markBoard[tmpX][tmpY] = true;
              markStone++;
              if (move1X(tmp) == 0x1f) tmp = tmp ^ 0xffc00 ^ (XY2pos(tmpX, tmpY) << 10);
              else {
                tmp = tmp ^ 0x3ff ^ XY2pos(tmpX, tmpY);
                break;
              }
            }
          }
          ThreatList[tCnt - 1] = tmp;
        }
      }
    }
  } // ThreatList 탐색 완료
  if (tCnt == 0) return 0; // threat window가 없다면
  if (tCnt == 1) {
    if (move2X(ThreatList[0]) == 0x1f) { // threat window내 빈 칸이 1개라면 막아야함
      ForcedMove[fidx++] = ((INVALID_MOVE ^ 0xffc00) | (move1pos(ThreatList[0]) << 10)); // ForcedMove에 넣어주면 됨
      return 1;
    }
    // 1. Threat 후보의 첫 번쨰를 메꿨을 때 Threat이 제거되었는가.    
    myBoard[move1X(ThreatList[0])][move1Y(ThreatList[0])] = player; // 내 수로 막아둠(myBoard에 해야함)
    if (IsNoThreat((INVALID_MOVE ^ 0x3ff) | move2pos(ThreatList[0]), opponent))
      ForcedMove[fidx++] = (INVALID_MOVE ^ 0xffc00 ^ (move1pos(ThreatList[0]) << 10));
    myBoard[move1X(ThreatList[0])][move1Y(ThreatList[0])] = EMPTY; // 해제
  
    // 2. Threat 후보의 두 번째를 메꿨을 때 Threat이 제거되었는가.    
    myBoard[move2X(ThreatList[0])][move2Y(ThreatList[0])] = player; // 내 수로 막아둠
    if (IsNoThreat(INVALID_MOVE ^ 0x3ff ^ move1pos(ThreatList[0]), opponent))
      ForcedMove[fidx++] = (INVALID_MOVE ^ 0xffc00 ^ (move2pos(ThreatList[0]) << 10));
    myBoard[move2X(ThreatList[0])][move2Y(ThreatList[0])] = EMPTY; // 해제
    return 1;
  }
  // tCnt == 2
  POSITION pos1, pos2;
  MOVES oppoMove = INVALID_MOVE;
  for (int i = 0; i < 2; i++) {
    if (i == 0) {
      pos1 = move1pos(ThreatList[0]);
      oppoMove = (oppoMove ^ 0xffc00) | ((move2pos(ThreatList[0]) << 10));
    }
    else {
      pos1 = move2pos(ThreatList[0]);
      oppoMove = (oppoMove ^ 0xffc00) | ((move1pos(ThreatList[0]) << 10));
    }
    if (posX(pos1) == 0x1f) continue;
    for (int j = 0; j < 2; j++) {
      if (j == 0) {
        pos2 = move1pos(ThreatList[1]);
        oppoMove = (oppoMove ^ 0x1f) | move2pos(ThreatList[1]);
      }
      else {
        pos2 = move2pos(ThreatList[1]);
        oppoMove = (oppoMove ^ 0x1f) | move1pos(ThreatList[1]);
      }
      if (posX(pos2) == 0x1f) continue;
      myBoard[posX(pos1)][posY(pos1)] = player;
      myBoard[posX(pos2)][posY(pos2)] = player;
      if (IsNoThreat(oppoMove, opponent)) // threat 후보 중에서 하나를 메꾸니 Threat이 없어졌다면 이는 올바른 착수
        ForcedMove[fidx++] = (pos2move(pos1, pos2));
      myBoard[posX(pos1)][posY(pos1)] = EMPTY;
      myBoard[posX(pos2)][posY(pos2)] = EMPTY; // 원상복귀
    }
  }
  return tCnt;
}

// cur를 root로 하는 Tree를 Full Tree로 만듬
// cur에 해당하는 착수는 이미 진행된 채로 call됨
// cur->FullSearched는 변경이 된채로 return됨
bool MinimaxTree::expFullTree(Node* cur) {
  if (clock() - st_time > limit)
    return false;
  for (auto& ch : cur->children) {
    // 자식의 depth가 올바르다면
    if (ch->depth + 1 == cur->depth) {
      // 자식이 fullSearched라면
      if (ch->fullSearched) continue;
      setMove(ch->myMove, ch->player);
      if (!expFullTree(ch)) { // child를 full tree로 만들다가 시간초과 발생시
        setMove(ch->myMove, EMPTY);
        return false;
      }
      // 시간초과가 발생하지 않았다면 cur score의 값 변경
      setMove(ch->myMove, EMPTY);
      if (cur->score[cur->depth] == INVALID_SCORE)
        cur->score[cur->depth] = ch->score[ch->depth];
      else {
        // 현재 cur에서 가지고있는 값보다 ch에서의 값이 더 좋다면
        // cur의 홀수 depth일 때는 자식의 최댓값, 짝수 depth일 때는 자식의 최솟값
        if (((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) < score2cmp(ch->score[ch->depth]))
          cur->score[cur->depth] = ch->score[ch->depth];
        else if (!((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) > score2cmp(ch->score[ch->depth]))
          cur->score[cur->depth] = ch->score[ch->depth];
      }
      // 홀수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
      if (((cur->depth) & 1) and cur->score[cur->depth] == MY_INF) {
        cur->fullSearched = true;
        return true;
      }
      // 짝수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
      else if (!((cur->depth) & 1) and cur->score[cur->depth] == OP_INF) {
        cur->fullSearched = true;
        return true;
      }
      continue;
    }
    // depth가 일치하지 않으니 depth를 일치시킴
    ch->depth = cur->depth - 1;
    ch->fullSearched = false;
    setMove(ch->myMove, ch->player);
    // 시간초과 발생시
    if (!expFullTree(ch)) {
      setMove(ch->myMove, EMPTY);
      return false;
    }
    // 시간초과가 발생하지 않았다면 cur score의 값 변경
    setMove(ch->myMove, EMPTY);
    if (cur->score[cur->depth] == INVALID_SCORE)
      cur->score[cur->depth] = ch->score[ch->depth];
    else {
      //assert(cur->depth - ch->depth == 1);
      // 현재 cur에서 가지고있는 값보다 ch에서의 값이 더 좋다면
      // cur의 홀수 depth일 때는 자식의 최댓값, 짝수 depth일 때는 자식의 최솟값
      if (((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) < score2cmp(ch->score[ch->depth]))
        cur->score[cur->depth] = ch->score[ch->depth];
      else if (!((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) > score2cmp(ch->score[ch->depth]))
        cur->score[cur->depth] = ch->score[ch->depth];
    }
    // 홀수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
    if (((cur->depth) & 1) and cur->score[cur->depth] == MY_INF) {
      cur->fullSearched = true;
      return true;
    }
    // 짝수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
    else if (!((cur->depth) & 1) and cur->score[cur->depth] == OP_INF) {
      cur->fullSearched = true;
      return true;
    }
  }
  cur->fullSearched = true;
  return true;
}

// cur를 root로 하는 Tree의 depth를 1 증가시킴
// 이 때 cur의 착수는 이미 이루어진 상태로 도달함
// cur Node의 depth값 증가 및 FullSearched = false는 이미 이루어진 상태로 도달함
// full tree임은 보장되어있어야 함
// FullSearched를 return 전에 true로 만듬
bool MinimaxTree::incDepth(Node* cur) { 
  if (clock() - st_time > limit * CLOCKS_PER_SEC)
    return false;
  // 무조건 졌거나 무조건 이겼을 경우 굳이 더 내려가지 않는다.
  if (cur->score[cur->depth - 1] == MY_INF) {
    cur->fullSearched = true;
    cur->score[cur->depth] = OP_INF;
    return true;
  }
  if (cur->score[cur->depth - 1] == OP_INF) {
    cur->fullSearched = true;
    cur->score[cur->depth] = MY_INF;
    return true;
  }
  if (cur->children.empty()) { // leaf이면 leaf 확장한 후 return
    expLeaf(cur); // cur treeVal은 알아서 갱신이 됨
    cur->fullSearched = true;
    return true;
  }
  // leaf가 아니라면 재귀적으로 처리
  for (auto& ch : cur->children) {
    setMove(ch->myMove, ch->player); // 일단 자식의 수를 둬놓고 따라들어감
    ch->depth += 1;
    ch->fullSearched = false;
    if (!incDepth(ch)) { // 시간초과 발생시
      setMove(ch->myMove, EMPTY);
      return false;
    }
    if (cur->score[cur->depth] == INVALID_SCORE)
      cur->score[cur->depth] = ch->score[ch->depth];
    else {
      if (((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) < score2cmp(ch->score[ch->depth]))
        cur->score[cur->depth] = ch->score[ch->depth];
      else if (!((cur->depth) & 1) and score2cmp(cur->score[cur->depth]) > score2cmp(ch->score[ch->depth]))
        cur->score[cur->depth] = ch->score[ch->depth];
    }
    setMove(ch->myMove, EMPTY);
    // 홀수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
    if (((cur->depth) & 1) and cur->score[cur->depth] == MY_INF) {
      cur->fullSearched = true;
      return true;
    }
    // 짝수 depth이고 MAX이면(즉 상대 입장에서 필승 수가 있다면)
    else if (!((cur->depth) & 1) and cur->score[cur->depth] == OP_INF) {
      cur->fullSearched = true;
      return true;
    }
  }
  cur->fullSearched = true;
  return true;
}
// 현재 tree를 full tree로 확장 후 depth를 한 단계 늘림
// root의 착수는 이미 이루어져있음.(상대가 수를 뒀을 떄 보드를 갱신할 것이므로)
bool MinimaxTree::expTree() {
  // 1. 우선 Full Tree로 만듬. 시간초과 났을 경우 false 반환
  if (!root->fullSearched) { // root가 fullSearched가 아니라면
    if (!expFullTree(root))
      return false;
  }
  // 어차피 필패 or 필승이면 확장을 하지 않음
  if (root->score[root->depth] == MY_INF or root->score[root->depth] == OP_INF) {
    return true;
  }
  // 이제 depth는 다 맞춰짐
  root->depth += 1; // depth 1 추가. 즉 leaf를 한 단계 달 것임
  root->fullSearched = false; // 아직 추가된 depth에 대한 탐색은 이루어지지 않음
  if (!incDepth(root)) // 시간초과 발생시
    return false;
  root->fullSearched = true;
  return true;
}

// 여기서 depth 값을 바꾸지는 말고 leaf에 다음 수를 부착하고 leaf의 score[1] 변경
// 시간 초과는 체크하지 않는다.
bool MinimaxTree::expLeaf(Node* leaf) {
  int player = 3 ^ leaf->player; // leaf의 다음 수를 진행할 사람이 누군지를 의미
  // 1. 현재 6목을 만들 수 있는가?
  MOVES con6 = Con6Move(leaf->opMove, player);
  if (move1X(con6) != 0x1f) {
    Node* ch = new Node(con6, leaf->myMove, player, MY_INF);
    leaf->children.push_back(ch);
    leaf->score[1] = MY_INF;
    return true;
  }
  // 2. 반드시 막아야하는 Threat이 존재하는가?
  MOVES ForcedMove[4] = { INVALID_MOVE,INVALID_MOVE,INVALID_MOVE,INVALID_MOVE };
  int tCnt = GetForcedMove(leaf->myMove, player, ForcedMove);

  if (tCnt == 1) {
    MOVES BestMove[LEAF_NUM];
    ull BestScore[LEAF_NUM];
    int len = 0;
    for (int i = 0; i < LEAF_NUM; i++) {
      BestMove[i] = INVALID_MOVE;
      BestScore[i] = INVALID_SCORE;
    }   
    for (int ii = 0; ii < 4; ii++) {
      if (ForcedMove[ii] == INVALID_MOVE) break;
      POSITION pos1 = move1pos(ForcedMove[ii]); // 뒤의 것은 {-1,-1}이므로 의미가 없음.
      ull score1 = eval1(pos1, player);
      // leaf의 다음 수를 진행할 사람이 player이다.
      setPosition(pos1, player);
      for (int i = 0; i < BOARD_SZ; i++) {
        for (int j = 0; j < BOARD_SZ; j++) {
          POSITION pos2 = XY2pos(i, j);
          if (myBoard[i][j] != EMPTY) continue;
          ull fin_score = score1 + eval1(pos2, player);
          int pScore = (int)(leaf->score[0] & 0xffffffffu) + (int)(fin_score >> 32);
          int opScore = (int)(leaf->score[0] >> 32) - (int)(fin_score & 0xffffffffu) - threatBonus;  // threat은 제거됐을테니 빼준다.
          if (len == 0) {
            BestScore[len] = bw2score(pScore, opScore);
            BestMove[len++] = pos2move(pos1, pos2);
            continue;
          }
          int convert_score = (int)(pScore - SCORE_FACTOR * opScore);
          if (convert_score <= score2cmp(BestScore[len - 1])) {
            if (len == LEAF_NUM) continue;
            BestScore[len] = bw2score(pScore, opScore);
            BestMove[len++] = pos2move(pos1, pos2);
            continue;
          }
          int idx = len - 1;
          while (idx >= 0 and convert_score > score2cmp(BestScore[idx])) {
            if (idx != LEAF_NUM - 1) {
              BestScore[idx + 1] = BestScore[idx];
              BestMove[idx + 1] = BestMove[idx];
            }
            idx--;
          }
          BestScore[idx + 1] = bw2score(pScore, opScore);
          BestMove[idx + 1] = pos2move(pos1, pos2);

        }
      }
      setPosition(pos1, EMPTY);
    }
    // 만약 적합한 수가 하나도 없었다면 leaf 입장에서 필승
    if (len == 0) {
      Node* ch = new Node(getArbitraryMoves(player), leaf->myMove, player, OP_INF);
      leaf->children.push_back(ch);
      leaf->score[1] = OP_INF;
      return true;
    }
    // leaf 입장에서, 다음 사람은 move 중에서 자신에게 가장 이득인 수를 둘 것이므로
    leaf->score[1] = BestScore[0];
    // 이제 set에 들어있는 후보군을 leaf에 장착하기만 하면 됨
    for (int i = 0; i < len; i++) {
      Node* tmp = new Node(BestMove[i], leaf->myMove, player, BestScore[i]);
      leaf->children.push_back(tmp);
    }
    return true;
  }
  if (tCnt == 2) {
    if (ForcedMove[0] == INVALID_MOVE) { // threat이 있는데 금수로 인해 막을 수 있는 방법이 없는 경우
      Node* ch = new Node(getArbitraryMoves(player), leaf->myMove, player, OP_INF);
      leaf->children.push_back(ch);
      leaf->score[1] = OP_INF;
      return true;
    }
    // 상대는 자신에게 가장 유리한 점수를 택할 것이다.
    ull leaf_next = INVALID_SCORE;
    for (int ii = 0; ii < 4; ii++) {
      if (ForcedMove[ii] == INVALID_MOVE) break;
      ull score = eval2(ForcedMove[ii], player);
      int pScore = (int)(leaf->score[0] & 0xffffffffu) + (int)(score >> 32);
      int opScore = (int)(leaf->score[0] >> 32) - (int)(score & 0xffffffffu) - 2 * threatBonus;  // threat은 제거됐을테니 빼준다.
      if (leaf_next == INVALID_SCORE) leaf_next = bw2score(pScore, opScore);
      else {
        if (score2cmp(leaf_next) < pScore - SCORE_FACTOR * opScore)
          leaf_next = bw2score(pScore, opScore);
      }
      Node* tmp = new Node(ForcedMove[ii], leaf->myMove, player, bw2score(pScore, opScore));  
      leaf->children.push_back(tmp);
    }
    leaf->score[1] = leaf_next;
    return true;
  }
  if (tCnt == 3) {
    Node* tmp = new Node(ForcedMove[0], leaf->myMove, player, OP_INF);
    leaf->children.push_back(tmp);
    leaf->score[1] = OP_INF;
    return true;
  }
  // tCnt == 0
  MOVES BestMove[LEAF_NUM];
  ull BestScore[LEAF_NUM];
  for (int i = 0; i < LEAF_NUM; i++) {
    BestMove[i] = INVALID_MOVE;
    BestScore[i] = INVALID_SCORE;
  }
  GetBestMove(player, BestMove, BestScore);
  if (BestMove[0] == INVALID_MOVE) { // 웬만하면 안생길 일
    Node* ch = new Node(getArbitraryMoves(player), leaf->myMove, player, OP_INF);
    leaf->children.push_back(ch);
    leaf->score[1] = OP_INF;
    return true;
  }
  for (int i = 0; i < LEAF_NUM; i++) {
    if (BestMove[i] == INVALID_MOVE) break;
    MOVES myMove = BestMove[i];
    ull score = BestScore[i];
    int pScore = (int)(leaf->score[0] & 0xffffffffu) + (int)(score >> 32);
    int opScore = (int)(leaf->score[0] >> 32) - (int)(score & 0xffffffffu);
    if (i == 0)
      leaf->score[1] = bw2score(pScore, opScore); // 상대의 BestMove로 대입해둠
    
    Node* tmp = new Node(myMove, leaf->myMove, player, bw2score(pScore, opScore));
    int dx[4] = { 1,0,1,1 };
    int dy[4] = { 0,1,1,-1 };   
    leaf->children.push_back(tmp);
  }
  return true;
}