package bgu.spl.net.srv;

import java.io.Serializable;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;

public class StompFrame implements Serializable{
    private final ConcurrentHashMap<String, String> headerMap;
    private final String command;
    private final String body;

    // assume protocol verify correct headers and command.
    public StompFrame(String command, String body){
        this.headerMap = new ConcurrentHashMap<>();
        this.command = command;
        this.body = body;
    }

    public boolean addHeader(String key, String value){
        if (key != null && value != null){
            headerMap.put(key, value);
            return true;
        }
        return false;
    }
}