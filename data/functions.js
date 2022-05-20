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

setInterval(function() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            jsonData = JSON.parse(xhttp.responseText);
            console.log(jsonData);
            leaderboard(jsonData);
        }
    };
    xhttp.open("GET", "/points", true);
    xhttp.send();
}, 1000);