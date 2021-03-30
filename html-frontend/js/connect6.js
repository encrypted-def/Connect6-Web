var c = document.getElementById("board");
var ctx = c.getContext("2d");
var width = 600;
var height = 600;
var radius = 12;
var blank = 12;
var turn = 1; // 1 player 2 computer
var gamestart = false;
var isfirst = false;
var cnt = 0;
var sendmove = "";
var mytotmove = 0;
var websocket = new WebSocket("ws://localhost:8765"); // server ip

websocket.onmessage = function(evt){
    recvdata(evt)
  };


var boardArray = new Array(19); 
for (var i = 0; i < 19; i++) {
    boardArray[i] = new Array(19);
    for (j = 0; j < 19; j++) { 
    boardArray[i][j] = 0;
  }
}

var recentx0 = -1;
var recentx1 = -1;
var recenty0 = -1;
var recenty1 = -1;

updateBoard();
c.addEventListener('mousemove', moveEvent);
c.addEventListener('mousedown', downEvent);

$("#slider1").val(0);
$("#slider2").val(1);


$("#slider1").on("input change", function(){
  if($("#slider1").val() == 0){
    $(".info1").text("You First");
    turn = 1;
  }
  else{
    $(".info1").text("Computer First");
    turn = 2;
  }
});

$("#slider2").on("input change", function(){
    if($("#slider2").val() == 1)
      $(".info2").text("Level 1 - 컴퓨터는 한 수 앞까지 생각합니다. 연산이 오래 걸리지 않고 거의 바로 착수합니다.");
    if($("#slider2").val() == 2)
      $(".info2").text("Level 2 - 컴퓨터는 세 수 앞까지 생각합니다. 연산이 오래 걸리지 않고 거의 바로 착수합니다.");
    if($("#slider2").val() == 3)
      $(".info2").text("Level 3 - 컴퓨터는 최대 여섯 수 앞까지 생각합니다. 컴퓨터는 각 수를 최대 4초까지 생각합니다.");
    if($("#slider2").val() == 4)
      $(".info2").text("Level 4 - 컴퓨터는 최대 여덟 수 앞까지 생각합니다. 컴퓨터는 각 수를 최대 6초까지 생각합니다.");
    if($("#slider2").val() == 5)
      $(".info2").text("Level 5 - 컴퓨터는 최대 열 수 앞까지 생각합니다. 컴퓨터는 각 수를 최대 10초까지 생각합니다.");  
});

function moveEvent(evt){
  if(turn == 1 && gamestart){
    var mousePos = getMousePos(c, evt);
    drawNotClicked(mousePos.x, mousePos.y);  
  }
}

var tmpx0 = -1;
var tmpx1 = -1;
var tmpy0 = -1;
var tmpy1 = -1;    
    
function downEvent(evt){
  if(turn == 1 && gamestart){
    var mousePos = getMousePos(c, evt);
    resultPos = getMouseRoundPos(mousePos.x, mousePos.y);
    if (resultPos.x > -1 && resultPos.x < 19 && resultPos.y > -1
     && resultPos.y < 19 && boardArray[resultPos.x][resultPos.y] == 0){
      boardArray[resultPos.x][resultPos.y] = turn;
      if(cnt == 0){
        tmpx0 = resultPos.x;
        tmpy0 = resultPos.y;
        sendmove = resultPos.x + " " + resultPos.y;
      }
      else{        
        tmpx1 = resultPos.x;
        tmpy1 = resultPos.y;
        sendmove += " " + resultPos.x + " " + resultPos.y;        
      }
      cnt++;
      var sended = false;
      if(isfirst && cnt == 1){
        websocket.send(sendmove);
        sended = true;
        isfirst = false;
        recentx0 = tmpx0;
        recenty0 = tmpy0;
      }
      if(!isfirst && cnt == 2){
        websocket.send(sendmove);
        sended = true;   
        recentx0 = tmpx0;
        recenty0 = tmpy0;
        recentx1 = tmpx1;
        recenty1 = tmpy1;
      }
      if(sended){
        updateBoard();
        if(checkwin(1)){
          $("#status").val("축하합니다. 당신의 승리입니다.");
          sendmove = "";
          clearVariables();
        }
        else{
          $("#status").val("컴퓨터의 턴을 기다리는 중입니다.");
          cnt = 0;
          sendmove = "";
          turn = 2;
        }        
      }
      else{
        updateBoard();
      }

    }
  }
}

$('#start').click(function(){
  websocket.onmessage = function(evt){
    recvdata(evt)
  };
  $('#slider1').attr('disabled', true);
  $('#slider2').attr('disabled', true);
  $('#start').attr('disabled', true);
  var setting = $("#slider2").val() + " ";
  if(turn == 1){
    setting += "HUMAN";
    isfirst = true;
    $("#status").val("당신의 턴입니다.");
  }
  else{
    setting += "AI";
    isfirst = false;
    $("#status").val("컴퓨터의 턴을 기다리는 중입니다.");
  }
  websocket.send(setting);

  gamestart = true;  
});

function clearVariables(){
  for (var i = 0; i < 19; i++) {
    boardArray[i] = new Array(19);
      for (j = 0; j < 19; j++) { 
      boardArray[i][j] = 0;
    }
  }
  $("#slider1").val(0);  
  $("#slider2").val(1);
  $('#slider1').attr('disabled', false);
  $('#slider2').attr('disabled', false);
  $('#start').attr('disabled', false);
  $(".info1").text("You First");
  $(".info2").text("Level 1 - 컴퓨터는 한 수 앞까지 생각합니다. 연산이 오래 걸리지 않고 거의 바로 착수합니다.");
  gamestart = false;
  turn = 1;
  cnt = 0;
  mytotmove = 0;
}

function updateBoard(){
  // board fill color
  ctx.lineWidth = 1;
  ctx.fillStyle="#ffcc66";
  ctx.fillRect(0, 0, width, height);

  // board draw line
  ctx.strokeStyle="#333300";
  ctx.fillStyle="#333300";
  for (i = 0; i < 19; i++) { 
    // horizontal line draw
    ctx.beginPath();
    ctx.moveTo(blank + i * 32, blank);
    ctx.lineTo(blank + i * 32, height - blank);
    ctx.stroke();

    // vertical line draw
    ctx.beginPath();
    ctx.moveTo(blank, blank + i * 32);
    ctx.lineTo(height - blank, blank + i * 32);
    ctx.stroke();
  }

  // board draw point
  var circleRadius = 3;
  for (i = 0; i < 3; i++) { 
    for (j = 0; j < 3; j++) { 
      // board circle draw
      ctx.beginPath();
      ctx.arc(blank + 3 * 32 + i * 6 * 32, blank + 3 * 32  + j * 6 * 32, circleRadius, 0, 2*Math.PI);
      ctx.fill();
      ctx.stroke();
    }
  }

  // board draw clicked
  for (i = 0; i < 19; i++) { 
    for (j = 0; j < 19; j++) {
      if (boardArray[i][j] == 1) {
        ctx.beginPath();
        if (i == recentx0 && j == recenty0) {ctx.strokeStyle="#4aa8d8"; ctx.lineWidth=3;}
        else if(i == recentx1 && j == recenty1) {ctx.strokeStyle="#4aa8d8"; ctx.lineWidth=3;}
        else {ctx.strokeStyle="#000000";   ctx.lineWidth = 1;}
        ctx.fillStyle="#000000";
        ctx.arc(blank + i * 32, blank + j * 32, radius, 0, 2*Math.PI);
        ctx.fill();
        ctx.stroke();
      } else if (boardArray[i][j] == 2){
        ctx.beginPath();
        if(i == recentx0 && j == recenty0) {ctx.strokeStyle="#000077";  ctx.lineWidth=3;}
        else if(i == recentx1 && j == recenty1) {ctx.strokeStyle="#000077";  ctx.lineWidth=3;}
        else {ctx.strokeStyle="#ffffff"; ctx.lineWidth = 1;}
        ctx.fillStyle="#ffffff";
        ctx.arc(blank + i * 32, blank + j * 32, radius, 0, 2*Math.PI);
        ctx.fill();
        ctx.stroke();
      } else if (boardArray[i][j] == 3){
        ctx.beginPath();
        ctx.strokeStyle="#ff0000";
        ctx.fillStyle="#ff0000";
        ctx.rect(blank + i * 32 - radius, blank + j * 32 - radius, 2*radius, 2*radius);
        ctx.fill();
        ctx.stroke();
      }
    }
  }
}

//////// websocket ///////////////




function recvdata(evt){
  var query = evt.data;
  var query_chunk = query.split(' ');
  if(query_chunk[0] == "BLOCK"){
    var x = Number(query_chunk[1]);
    var y = Number(query_chunk[2]);
    boardArray[x][y] = 3;
    updateBoard();
  }
  else if(query_chunk[0] == "STATISTIC"){
    $("#statistic1").val("Level 1 - " + query_chunk[1] + "승 " + query_chunk[2] + "패");
    $("#statistic2").val("Level 2 - " + query_chunk[3] + "승 " + query_chunk[4] + "패");
    $("#statistic3").val("Level 3 - " + query_chunk[5] + "승 " + query_chunk[6] + "패");
    $("#statistic4").val("Level 4 - " + query_chunk[7] + "승 " + query_chunk[8] + "패");
    $("#statistic5").val("Level 5 - " + query_chunk[9] + "승 " + query_chunk[10] + "패");
  }
  else{
    if(query_chunk.length == 2){
      var x = Number(query_chunk[0]);
      var y = Number(query_chunk[1]);
      boardArray[x][y] = 2;
      recentx0 = x;
      recenty0 = y;
    }
    else{
      var x0 = Number(query_chunk[0]);
      var y0 = Number(query_chunk[1]);
      var x1 = Number(query_chunk[2]);
      var y1 = Number(query_chunk[3]);      
      boardArray[x0][y0] = 2;
      boardArray[x1][y1] = 2;       
      recentx0 = x0;
      recenty0 = y0;
      recentx1 = x1;
      recenty1 = y1;
      mytotmove += 2;
    }
    updateBoard();
    if(checkwin(2)){
      $("#status").val("컴퓨터의 승리입니다.");      
      clearVariables();
    }
    else if(mytotmove >= 170){ // 판을 얼추 다 채웠을 경우
      $("#status").val("무승부입니다.");
      clearVariables();
    }    
    else{
      turn = 1;
      $("#status").val("당신의 턴입니다.");
    }
  }
}

////////////////////////////////

function checkwin(player){
  var dx = [1,1,1,0];
  var dy = [-1,0,1,1];
  for(i = 0; i < 19; i++){
    for(j = 0; j < 19; j++){
      for(dir = 0; dir < 4; dir++){
        if(0 > i+5*dx[dir] || 18 < i+5*dx[dir] || 0 > j+5*dx[dir] || 18 < j+5*dy[dir]) continue;
        var iswin = true;
        for(k = 0; k < 6; k++){
          if(boardArray[i+k*dx[dir]][j+k*dy[dir]] != player){
            iswin = false;
            break;
          }
        }        
        if(iswin){
          for(k = 0; k < 6; k++){
            ctx.beginPath();
            var nx = i + k * dx[dir];
            var ny = j + k * dy[dir];
            if(player == 1){
              ctx.fillStyle="#000000";
              ctx.strokeStyle="#4aa8d8";
            }
            else{
              ctx.fillStyle="#FFFFFF";
              ctx.strokeStyle="#000077";
            }
            ctx.lineWidth = 3;            
            ctx.arc(blank + nx * 32, blank + ny * 32, radius, 0, 2*Math.PI);
            ctx.fill();
            ctx.stroke();
          }
          return true;
        }
      }
    }
  }
  return false;
}


/* Mouse Event */
function getMousePos(canvas, evt) {
  var rect = canvas.getBoundingClientRect();
  return {
    x: evt.clientX - rect.left,
    y: evt.clientY - rect.top
  };
}

function getMouseRoundPos(xPos, yPos){
  var x = (xPos - blank) / 32;
  var resultX = Math.round(x);
  var y = (yPos - blank) / 32;
  var resultY = Math.round(y);

  return {
    x: resultX,
    y: resultY
  };
}

function drawNotClicked(xPos, yPos){
  resultPos = getMouseRoundPos(xPos, yPos);

  if (resultPos.x > -1 && resultPos.x < 19 && resultPos.y > -1
   && resultPos.y < 19 && boardArray[resultPos.x][resultPos.y] == 0){
    updateBoard();
    ctx.beginPath();
    ctx.globalAlpha=0.8;
    if (turn < 2) {
      ctx.strokeStyle="#000000";
      ctx.fillStyle="#000000";
    } else {
      ctx.strokeStyle="#ffffff";
      ctx.fillStyle="#ffffff";  
    }
    ctx.arc(blank + resultPos.x * 32, blank + resultPos.y * 32, radius, 0, 2*Math.PI);
    ctx.fill();
    ctx.stroke();
    ctx.globalAlpha=1;
  }
};



