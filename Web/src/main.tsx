import App from "./App";
import React from "react";
import ReactDOM from "react-dom/client";
import "./main.css";

console.log("WARNING!!! This is only for local communication.  To use the internet version, go to jjthejjpro.com and click on the Monster Controller tab.");

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
    <React.StrictMode>
        <App />
    </React.StrictMode>,
);
