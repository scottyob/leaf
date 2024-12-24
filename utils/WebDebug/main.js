// import {Chart} from 'chart.js';
// import 'chartjs-adapter-luxon';
// import ChartStreaming from 'chartjs-plugin-streaming';

Chart.register(ChartStreaming);
console.log("ChartStreaming", ChartStreaming);

// Telemetry Config
const graph_telemetry = {
  Barometer: [
    {
      label: "Pressure",
      borderColor: "red",
      backgroundColor: "red",
      yAxisID: "y2",
      key: "bp",
    },
    {
      label: "Climb Rate Filtered",
      borderColor: "green",
      backgroundColor: "green",
      key: "bcf",
    },
  ],
};

// Create a mapping of keys to data
for (const graph of Object.values(graph_telemetry)) {
  for (const dataset of graph) {
    dataset.borderWidth = 1;
    dataset.pointRadius = 1;
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
    data: {
      datasets: datasets.map((data) => ({
        ...data,
        data: [],
      })),
    },
    options: {
      scales: {
        x: {
          type: "realtime",
          realtime: {
            refresh: 60,
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
