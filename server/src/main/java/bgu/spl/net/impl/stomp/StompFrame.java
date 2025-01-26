package bgu.spl.net.impl.stomp;

import java.io.Serializable;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;

public class StompFrame implements Serializable {
    private final ConcurrentHashMap<String, String> headerMap;
    private String command;
    private String body;

    // assume protocol verify correct headers and command.
    public StompFrame(String command, String body) {
        this.headerMap = new ConcurrentHashMap<>();
        this.command = command;
        this.body = body;
    }

    public StompFrame(byte[] bytes) {
        this.headerMap = new ConcurrentHashMap<>();
        String message = new String(bytes);
        String[] arr = message.split("\n\n");
        String[] content = arr[0].split("\n");

        this.command = content[0];

        for (int i = 1; i < content.length; i++) {
            String key = content[i].split(":")[0];
            String val = content[i].split(":")[1];
            addHeader(key, val);
        }

        this.body = arr[1];
    }

    public boolean addHeader(String key, String value) {
        if (key != null && value != null) {
            headerMap.put(key, value);
            return true;
        }
        return false;
    }

    public String getCommand() {
        return command;
    }

    public ConcurrentHashMap<String, String> getHeaderMap() {
        return headerMap;
    }

    public String getBody() {
        return body;
    }

    @Override
    public String toString() {
        StringBuilder str = new StringBuilder();
        str.append(this.command + "\n");
        for (String key : this.headerMap.keySet()) {
            str.append(key + ":" + this.headerMap.get(key) + "\n");
        }

        str.append("\n");

        str.append(this.body);

        return str.toString();
    }
}