// import {Chart} from 'chart.js';
// import 'chartjs-adapter-luxon';
// import ChartStreaming from 'chartjs-plugin-streaming';

import { plugin as chartMouseHoverPlugin } from "./chartMouseHoverPlugin.js";

Chart.register(ChartStreaming);
console.log("ChartStreaming", ChartStreaming);

// Telemetry Config
const graph_telemetry = {
  Barometer: [
    {
      label: "Pressure",
      // color: "lightred",
      yAxisID: "y2",
      key: "bp",
    },
    {
      label: "Climb Rate Filtered",
      // color: "lightgreen",
      key: "bcf",
    },
  ],
  Altitude: [
    {
      label: "Barometer",
      key: "ba",
    },
    {
      label: "GPS (meters)",
      key: "ga",
      yAxisID: "y2",
    },
  ],
};

// Create a mapping of keys to data
for (const graph of Object.values(graph_telemetry)) {
  for (const dataset of graph) {
    dataset.borderWidth = 1;
    dataset.pointRadius = 1;
    if (dataset.color !== undefined) {
      dataset.borderColor = dataset.color;
      dataset.backgroundColor = dataset.color;
    }
    dataset.lastData = undefined;
  }
}

// Create a mapping on keys to their dataset objects
const graphData = {};
for (const graph of Object.values(graph_telemetry)) {
  for (const dataset of graph) {
    // Check if dataset.key is in the graph data.  If it's not
    // add a new list for it, otherwise append to the existing list
    if (!graphData[dataset.key]) {
      graphData[dataset.key] = [];
    }
    graphData[dataset.key].push(dataset);
  }
}

let linex = 0;

// Create chart.js charts for each graph
for (const [name, datasets] of Object.entries(graph_telemetry)) {
  // Create a mapping of datasets to their yAxisID
  const yAxisID = {};
  for (const dataset of datasets) {
    const yId = dataset.yAxisID ?? "y";
    yAxisID[yId] = dataset;
  }

  const config = {
    type: "line",
    plugins: [
      {
        colorschemes: {
          scheme: "tableau.Tableau10",
        },
      },
      {
        afterEvent: (chart, args) => {
          const { inChartArea } = args;
          const { type, x, y } = args.event;
          linex = x;
        },
        afterDraw: function (chart) {
          const ctx = chart.ctx;
          const xAxis = chart.scales.x;

          // Draw vertical line at x-axis value 4
          const xValue = linex; // xAxis.getPixelForValue(4);
          ctx.save();
          ctx.strokeStyle = "rgb(255, 99, 132)";
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.moveTo(xValue, 0);
          ctx.lineTo(xValue, chart.height);
          ctx.stroke();
          ctx.restore();
        },
      },
    ],
    data: {
      datasets: datasets.map((data) => ({
        ...data,
        data: [],
      })),
    },
    options: {
      hover: {
        mode: "index",
        intersect: false,
      },
      maintainAspectRatio: false,
      scales: {
        x: {
          type: "realtime",
          realtime: {
            refresh: 100,
            duration: 60000,
            onRefresh: (chart) => {
              // Loop through each dataset, Add the data to the graph
              for (let i = 0; i < datasets.length; i++) {
                const dataset = datasets[i];
                const lastData = dataset.lastData;
                if (lastData !== undefined) {
                  chart.data.datasets[i].data.push({
                    x: Date.now(),
                    y: lastData,
                  });
                  dataset.lastData = undefined;
                }
              }
            },
          },
        },
        y: {
          type: "linear",
          display: true,
          position: "left",
        },
        y2: {
          type: "linear",
          display: true,
          position: "right",
        },
      },
    },
  };

  // Create the chart
  const div = document.getElementById(name);
  const canvas = document.createElement("canvas");
  canvas.id = `${name}Canvas`;
  div.appendChild(canvas);
  // canvas.parentNode.style.height = '100px';
  const chart = new Chart(canvas, config);
}

let socket; // Websocket to get data on
let lastData = {}; // Last data received from the WebSocket

// Function to create a WebSocket connection
function createWebSocket() {
  // Close the existing WebSocket connection if it exists
  if (socket) {
    socket.close();
  }

  // Get the address from the input element
  const addressInput = document.getElementById("address");
  const address = addressInput.value;

  // Create a new WebSocket connection
  socket = new WebSocket(`ws://${address}/`);

  // Event listener for when the connection is opened
  socket.addEventListener("open", (event) => {
    console.log("WebSocket connection opened:", event);
  });

  // Event listener for when a message is received
  socket.addEventListener("message", (event) => {
    const data = event.data;
    // Split the key from the data using the ':' character
    const [key, value] = data.split(":");
    // console.log("Received data:", key, value);

    if (!key || !value) {
      console.error("Invalid data received:", data);
      return;
    }

    // Update the last data received globally
    const valueData = parseInt(value);
    lastData[key] = valueData;

    // Update the graph data, assume int for now
    graphData[key].forEach((dataset) => {
      dataset.lastData = valueData;
    });
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
