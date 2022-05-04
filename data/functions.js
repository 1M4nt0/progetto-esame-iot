function leaderboard() {
    var leaderboard = document.getElementById("leaderboard");
    var tbody = leaderboard.querySelector("tbody");
    var tbodyHtml = "";

    var player1 = { name: "Player1", TotWins: "4", score: "2.0" };
    var player2 = { name: "Player2", TotWins: "2", score: "2.1" };
    var player3 = { name: "Player3", TotWins: "0", score: "1.2" };
    var player4 = { name: "Player4", TotWins: "4", score: "0.1" };

    var players = [player1, player2, player3, player4];

    players.sort(function(a, b) {
        return Number(a.score) - Number(b.score);
    });

    for (var player of players) {
        tbodyHtml +=
            '<tr><td id="name">' +
            player.name +
            '</td><td id="speed" >' +
            player.score +
            '</td><td id="win" >' +
            player.TotWins +
            "</td></tr>";
    }

    tbody.innerHTML = tbodyHtml;
}

leaderboard();