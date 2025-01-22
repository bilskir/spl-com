package bgu.spl.net.srv;

import java.io.IOException;

import bgu.spl.net.api.MessageTransformer;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg, MessageTransformer<T> transformer);

    ConnectionHandler<T> disconnect(int connectionId);
}
