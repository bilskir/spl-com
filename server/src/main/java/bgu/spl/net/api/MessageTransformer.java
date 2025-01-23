package bgu.spl.net.api;

@FunctionalInterface
public interface MessageTransformer<T> {
    T transformMessage(T originalMessage,int subscriptionID, int messageID);
} 
