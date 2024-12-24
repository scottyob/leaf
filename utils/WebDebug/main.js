// import {Chart} from 'chart.js';
// import 'chartjs-adapter-luxon';
// import ChartStreaming from 'chartjs-plugin-streaming';

Chart.register(ChartStreaming);
console.log("ChartStreaming", ChartStreaming);

let baro_pressure = undefined; // Barometer pressure
let baro_climbrate_filtered = null; // Filtered climb rate

const config = {
  type: "line",
  data: {
    datasets: [
      {
        label: "Barometer Pressure",
        backgroundColor: "rgba(54, 162, 235, 0.5)",
        borderColor: "rgb(54, 162, 235)",
        cubicInterpolationMode: "monotone",
        // fill: true,
        data: [],
      },
      {
        label: "Barometer Climb Filtered",
        data: [],
      },
    ],
  },
  options: {
    scales: {
      x: {
        type: "realtime",
        realtime: {
          refresh: 30,
          onRefresh: (chart) => {
            chart.data.datasets[0].data.push({
              x: Date.now(),
              y: baro_pressure,
            });
            chart.data.datasets[1].data.push({
              x: Date.now(),
              y: baro_climbrate_filtered,
            });
            baro_pressure = undefined;
            baro_climbrate_filtered = undefined;
          },
        },
      },
    },
  },
};

// Get the baro div
const baroDiv = document.getElementById("baro");

// Create a new canvas element
const canvas = document.createElement("canvas");

// Set an id for the canvas if needed
canvas.id = "baroCanvas";

// Append the canvas to the baro div
baroDiv.appendChild(canvas);

// Create the chart using the new canvas element
const myChart = new Chart(canvas, config);

// Function to create a WebSocket connection
function createWebSocket() {
  // Get the address from the input element
  const addressInput = document.getElementById("address");
  const address = addressInput.value;

  // Create a new WebSocket connection
  const socket = new WebSocket(`ws://${address}/`);

  // Event listener for when the connection is opened
  socket.addEventListener("open", (event) => {
    console.log("WebSocket connection opened:", event);
  });

  // Event listener for when a message is received
  socket.addEventListener("message", (event) => {
    const data = event.data;
    // console.log("Received text data:", event.data);

    // If the data starts with "bf", it is a barometer reading
    // For filtered climb rate
    if (data.startsWith("bcf:")) {
      baro_climbrate_filtered = parseInt(data.slice(4));
      //   const baro = parseInt(data.slice(4));
      //   myChart.data.datasets[0].data.push({ x: Date.now(), y: baro });
    }
    if (data.startsWith("bp:")) {
      baro_pressure = parseInt(data.slice(3));
    }

    // if (data.startsWith("bp:")) {
    //   const baro = parseInt(data.slice(3));
    //   myChart.data.datasets[1].data.push({ x: Date.now(), y: baro });
    // }
    // myChart.update('none');
  });

  // Event listener for when the connection is closed
  socket.addEventListener("close", (event) => {
    console.log("WebSocket connection closed:", event);
  });

  // Event listener for when an error occurs
  socket.addEventListener("error", (event) => {
    console.error("WebSocket error:", event);
  });
}

// Example usage: Call createWebSocket when a button is clicked
document.getElementById("connect").addEventListener("click", createWebSocket);
