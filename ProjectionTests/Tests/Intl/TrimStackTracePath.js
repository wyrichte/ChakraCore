function TrimStackTracePath(line) {
    return line && line.replace(/\(.+ProjectionTests\\Tests.[^\\/]*./ig, "(");
}
