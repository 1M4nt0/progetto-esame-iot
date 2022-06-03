function leaderboard(jsonData, isMulti) {
    var leaderboard = document.getElementById("points-table");
    var tbody = leaderboard.querySelector("tbody");
    var tbodyHtml = "";

    players = jsonData["players"];
    if (isMulti) {
        players.sort(function(a, b) {
            return Number(b.time) - Number(a.time);
        });
    } else {
        players.sort(function(a, b) {
            return Number(a.time) - Number(b.time);
        });
    }

    index = 1;
    for (var player of players) {
        tbodyHtml +=
            '<tr id="player' +
            index +
            'row"><td class="position" >' +
            index +
            '</td><td class="name" >Giocatore ' +
            player.id +
            '</td><td class="speed" >' +
            player.time +
            '</td><td class="points" >' +
            player.points +
            "</td></tr>";
        index++;
    }

    tbody.innerHTML = tbodyHtml;
}

async function changePause() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            pause = JSON.parse(xhttp.responseText)["pause"];
            console.log(pause);
            setPause(!pause);
        }
    };
    xhttp.open("GET", "/pause", true);
    xhttp.send();
}

function setPause(pause) {
    var xhttp = new XMLHttpRequest();
    var url = "/pause?pause=" + (pause ? "1" : "0");
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            if (pause) {
                document.getElementById("pauseBtn").style.display = "none";
                document.getElementById("playBtn").style.display = "block";
            } else {
                document.getElementById("pauseBtn").style.display = "block";
                document.getElementById("playBtn").style.display = "none";
            }
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}

function setGamemode(gamemode) {
    var xhttp = new XMLHttpRequest();
    var url = "/gamemode?id=" + gamemode;
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {}
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}

function increasePlayers() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/players?number=increase", false);
    xhttp.send();
    try {
        xhr.send();
        if (xhr.status != 200) {
            console.log(`Error ${xhr.status}: ${xhr.statusText}`);
        } else {
            console.log(xhr.response);
        }
    } catch (err) {
        //alert("Request failed");
    }
}

function decreasePlayers() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/players?number=decrease", false);
    xhttp.send();
    try {
        xhr.send();
        if (xhr.status != 200) {
            console.log(`Error ${xhr.status}: ${xhr.statusText}`);
        } else {
            console.log(xhr.response);
        }
    } catch (err) {
        //alert("Request failed");
    }
}

function resetPoints() {
    var xhttp = new XMLHttpRequest();
    var url = "/reset";
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {}
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}