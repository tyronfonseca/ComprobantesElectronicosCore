import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import tailwindcss from "@tailwindcss/vite";
import path from "node:path";

export default defineConfig({
  plugins: [react(), tailwindcss()],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src"),
      "@cec-cr/core": path.resolve(__dirname, "../../bindings/typescript/src/index.ts")
    }
  },
  server: {
    port: 3000,
    open: true
  }
});
