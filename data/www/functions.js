function leaderboard(jsonData) {
    var leaderboard = document.getElementById("leaderboard");
    var tbody = leaderboard.querySelector("tbody");
    var tbodyHtml = "";

    players = jsonData["players"];
    players.sort(function(a, b) {
        return Number(a.time) - Number(b.time);
    });

    for (var player of players) {
        tbodyHtml +=
            '<tr id="player' +
            player.id +
            'row"><td class="name" >Giocatore ' +
            player.id +
            '</td><td class="speed" >' +
            player.time +
            '</td><td class="points" >' +
            player.points +
            "</td></tr>";
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
        if (this.readyState == 4 && this.status == 200) {}
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