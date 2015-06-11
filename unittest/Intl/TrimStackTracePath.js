function TrimStackTracePath(line) {
    return line && line.replace(/\(.+unittest.[^\\/]*./ig, "(");
}
