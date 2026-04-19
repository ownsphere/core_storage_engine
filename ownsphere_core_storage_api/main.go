package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
)

func runCommand(args ...string) (string, error) {
	cmd := exec.Command("/Users/apple/Documents/maxwell/ownsphere_core_storage_engine/build/engine", args...)
	cmd.Dir = "/Users/apple/Documents/maxwell/ownsphere_core_storage_engine"

	output, err := cmd.CombinedOutput()
	return string(output), err
}

func uploadHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseMultipartForm(10 << 20) // 10MB
	if err != nil {
		http.Error(w, "Failed to parse form", 400)
		return
	}

	file, handler, err := r.FormFile("file")
	if err != nil {
		http.Error(w, "File not found", 400)
		return
	}
	defer file.Close()

	// Save temp file
	tempPath := "/tmp/" + handler.Filename
	out, err := os.Create(tempPath)
	if err != nil {
		http.Error(w, "Failed to save file", 500)
		return
	}
	defer out.Close()

	_, err = io.Copy(out, file)
	if err != nil {
		http.Error(w, "Failed to write file", 500)
		return
	}

	// Call engine
	fileId := handler.Filename
	output, err := runCommand("store", tempPath, fileId)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}

	fmt.Fprintln(w, output)
}

func storeHandler(w http.ResponseWriter, r *http.Request) {
	file := r.URL.Query().Get("file")
	id := r.URL.Query().Get("id")

	out, err := runCommand("store", file, id)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}
	fmt.Fprintln(w, out)
}

func listHandler(w http.ResponseWriter, r *http.Request) {
	out, err := runCommand("list")
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}
	fmt.Fprintln(w, out)
}

func readHandler(w http.ResponseWriter, r *http.Request) {
	id := r.URL.Query().Get("id")
	outFile := r.URL.Query().Get("out")

	out, err := runCommand("read", id, outFile)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}
	fmt.Fprintln(w, out)
}

func deleteHandler(w http.ResponseWriter, r *http.Request) {
	id := r.URL.Query().Get("id")

	out, err := runCommand("delete", id)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}
	fmt.Fprintln(w, out)
}

func main() {
	http.HandleFunc("/store", storeHandler)
	http.HandleFunc("/list", listHandler)
	http.HandleFunc("/read", readHandler)
	http.HandleFunc("/delete", deleteHandler)
	http.HandleFunc("/upload", uploadHandler)

	fmt.Println("Server running on :8080")
	fmt.Println("Upload route registered") // ✅ ADD THIS

	err := http.ListenAndServe(":8080", nil)
	if err != nil {
		fmt.Println("Server error:", err)
	}
}
