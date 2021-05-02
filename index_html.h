const char html_page[] PROGMEM = R"(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>
        SonicBros - ESP8266
    </title>
    <style>
        html, body{
            margin: 0;
        }
        #rader {
            border: 1px solid black;
        }

        /* グリッド */
        .cross3x3 {
            display: grid;
            width: 150px;
            height: 150px;
            grid-template-columns: 1fr 1fr 1fr;
            grid-template-rows: 1fr 1fr 1fr;
        }

        .cross3x3-top-left {
            grid-row: 1 / 2;
            grid-column: 1 / 2;
        }

        .cross3x3-top {
            grid-row: 1 / 2;
            grid-column: 2 / 3;
        }

        .cross3x3-top-right {
            grid-row: 1 / 2;
            grid-column: 3 / 4;
        }

        .cross3x3-left {
            grid-row: 2 / 3;
            grid-column: 1 / 2;
        }

        .cross3x3-dummy-btn {
            grid-row: 2 / 3;
            grid-column: 2 / 3;
        }

        .cross3x3-right {
            grid-row: 2 / 3;
            grid-column: 3 / 4;
        }

        .cross3x3-bottom-left {
            grid-row: 3 / 4;
            grid-column: 1 / 2;
        }

        .cross3x3-bottom {
            grid-row: 3 / 4;
            grid-column: 2 / 3;
        }

        .cross3x3-bottom-right {
            grid-row: 3 / 4;
            grid-column: 3 / 4;
        }

        .div2btn {
            display: inline-block;
            background-color: azure;
            user-select: none;
            /* CSS3 */
            text-align: center;
            -moz-user-select: none;
            -webkit-user-select: none;
            -ms-user-select: none;
            user-select: none;
        }

        .cross-btn {
            height: 50px;
            width: 50;
            text-align: center;
            line-height: 50px;
        }

        .btn-shadow {
            filter: drop-shadow(0px 2px 0px grey);
        }

        .btn-shadow:active {
            transform: translateX(0px) translateY(2px);
            filter: drop-shadow(0px 0px 0px grey);
        }
        @media (min-aspect-ratio: 2/1){
            #rader {
                height: 100vh;
            }
        }

        @media (max-aspect-ratio: 2/1) {
            #rader {
                width: 100vw;
            }
        }

    </style>
</head>

<body onload='init_rader();'>
    <canvas id='rader' width='2000px' height='1000px'></canvas>

    <div class="cross3x3">
        <div class="cross3x3-top btn-shadow div2btn cross-btn" id="camera-up-btn">↑</div>
        <div class="cross3x3-left btn-shadow div2btn cross-btn" id="turn-left-btn">←</div>
        <div class="cross3x3-dummy-btn btn-shadow"></div>
        <div class="cross3x3-right btn-shadow div2btn cross-btn" id="turn-right-btn">→</div>
        <div class="cross3x3-bottom btn-shadow div2btn cross-btn" id="camera-down-btn">↓</div>
    </div>
    <script>
        const max_dist = 2000;
        let canvas = document.getElementById('rader');
        var ctx = canvas.getContext('2d');
        var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
        connection.onopen = function () {
            connection.send('Connect ' + new Date());
        };
        connection.onerror = function (error) {
            console.log('WebSocket Error ', error);
        };
        connection.onmessage = function (e) {
            console.log('Server: ', e.data);
            const obj = JSON.parse(e.data);
            draw_rader_beam(parseInt(obj.deg), parseInt(obj.dist));
        };
        function init_rader() {
            ctx.fillStyle = 'rgb(0,255,0)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = 'rgba(0,0,0,0.05)';
            for (let i = 0; i < 100; i++) {
                ctx.fillRect(0, 0, canvas.width, canvas.height);
            }
        }
        function draw_rader_beam(deg, dist) {
            let x = Math.cos(deg / 180 * Math.PI) * dist / max_dist * canvas.width / 2;
            x += (canvas.width / 2);
            let y = Math.sin(deg / 180 * Math.PI) * dist / max_dist * canvas.height;
            y = canvas.height - y;
            let max_x = Math.cos(deg / 180 * Math.PI) * canvas.width / 2;
            max_x += (canvas.width / 2);
            let max_y = Math.sin(deg / 180 * Math.PI) * canvas.height;
            max_y = canvas.height - max_y;

            ctx.fillStyle = 'rgba(0,0,0,0.05)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);

            ctx.strokeStyle = 'rgb(255,0,0)';
            ctx.lineWidth = 3;
            ctx.beginPath();
            ctx.moveTo(canvas.width / 2, canvas.height);
            ctx.lineTo(max_x, max_y);
            ctx.stroke();

            ctx.strokeStyle = 'rgb(0,255,0)';
            ctx.lineWidth = 3;
            ctx.beginPath();
            ctx.moveTo(canvas.width / 2, canvas.height);
            ctx.lineTo(x, y);
            ctx.stroke();
        }
        function send_cmd(cmd) {
            connection.send(cmd);
        }
    </script>
</body>


</html>
)";
