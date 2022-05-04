function leaderboard() {
    var leaderboard = document.getElementById("leaderboard");
    var tbody = leaderboard.querySelector("tbody");
    var tbodyHtml = "";

    var player1 = { id: "0", TotWins: "4", score: "2.0" };
    var player2 = { id: "1", TotWins: "2", score: "2.1" };
    var player3 = { id: "2", TotWins: "0", score: "1.2" };
    var player4 = { id: "3", TotWins: "4", score: "0.1" };

    var players = [player1, player2, player3, player4];

    players.sort(function(a, b) {
        return Number(a.score) - Number(b.score);
    });

    for (var player of players) {
        tbodyHtml +=
            '<tr id="player' +
            player.id +
            'row"><td class="name">' +
            player.name +
            '</td><td class="speed" >' +
            player.score +
            '</td><td class="points" >' +
            player.TotWins +
            "</td></tr>";
    }

    tbody.innerHTML = tbodyHtml;
}

leaderboard();

setInterval(function() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            data = JSON.parse(xhttp.responseText);
            document.getElementById("player" + data.playerID + "row").getElementsByClassName("points")[0].innerHTML =
                data.points;
        }
    };
    xhttp.open("GET", "/points", true);
    xhttp.send();
}, 2000);