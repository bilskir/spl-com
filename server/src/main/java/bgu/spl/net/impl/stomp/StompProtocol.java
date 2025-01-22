package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;


public class StompProtocol implements StompMessagingProtocol<StompFrame> {
    ConnectionsImpl<StompFrame> connections;
    private boolean shouldTerminate = false;
    private int connectionId;

    @Override
    public void start(int connectionId, Connections<StompFrame> connections, ConnectionHandler<StompFrame> handler) {
        this.connections = (ConnectionsImpl<StompFrame>) connections;
        this.connectionId = connectionId;
        this.connections.connect(this.connectionId, handler);
    }

    @Override
    public void process(StompFrame message) {
        switch (message.getCommand()) {
            case "CONNECT":{
                handleConnection(message);
                break;
            }
                
            case "DISCONNECT":{
                handleDisconnection(message);
                break;
            }

            case "SUBSCRIBE":{
                handleSubscription(message);
                break;
            }

            case "UNSUBSCRIBE":{
                handleUnsubscription(message);
                break;
            }

            case "SEND":{
                handleSend(message);
                break;
            }


            default:
                break;
        }
    }


    private void handleConnection(StompFrame message){
        String userName = message.getHeaderMap().get("login");
        String password = message.getHeaderMap().get("passcode");
        int res = connections.login(this.connectionId, userName, password);
        // Password incorrect
        if(res == -1){
            // Create ERROR frame
            System.out.println("wrong password");          
            StompFrame errorFrame = new StompFrame("ERROR","");
            errorFrame.addHeader("message", "wrong password");
            connections.send(this.connectionId, errorFrame);
            connections.disconnect(this.connectionId);
        }

        // User already logged in
        else if(res == -2){
            // Create Error frame
            System.out.println("user already logged in");
            StompFrame errorFrame = new StompFrame("ERROR","");
            errorFrame.addHeader("message", "user already logged in");
            connections.send(this.connectionId, errorFrame);
            connections.disconnect(this.connectionId);
        }

        // Logged in successfully
        else{
            StompFrame connectedFrame = new StompFrame("CONNECTED", "");
            System.out.println("success");
            connectedFrame.addHeader("version", "1.2");
            connections.send(this.connectionId, connectedFrame);
        }
    }

    private void handleDisconnection(StompFrame message){
        String reciptID = message.getHeaderMap().get("receipt");

        // logout from connections
        ConnectionHandler<StompFrame> handler = connections.disconnect(this.connectionId);


        // Send receipt to client
        StompFrame receiptFrame = new StompFrame("RECEIPT", "");
        receiptFrame.addHeader("receipt-id", reciptID);
        handler.send(receiptFrame);
    }

    private void handleSubscription(StompFrame message){
        String channel = message.getHeaderMap().get("destination");
        String subscriptionID = message.getHeaderMap().get("id");

        int result = connections.subscribe(channel, Integer.parseInt(subscriptionID), connectionId);
        
        // Client Couldn't subscribe
        if(result == -1){
            StompFrame errorFrame = new StompFrame("ERROR", "");
            errorFrame.addHeader("message", "Use already subscribed to this channel");
            connections.send(this.connectionId, errorFrame);
        }

    }

    private void handleUnsubscription(StompFrame message){
        String subscriptionID = message.getHeaderMap().get("id");
        connections.unsubscribe(connectionId, Integer.parseInt(subscriptionID));
    }
   

    private void handleSend(StompFrame message){
        String channel = message.getHeaderMap().get("destination");
        
        if(channel == null){
            StompFrame errorFrame = new StompFrame("ERROR", "The Message:\n----\n" + message.getCommand() + "\n" + message.getHeaderMap() + "\n "+ message.getBody() + "\n" + "Did not contain a destination header\nwhich is REQUIRED for message propagation.");
            errorFrame.addHeader("message", "malformed frame received");
            connections.send(this.connectionId, errorFrame);
        }
        else{
            int result = connections.send(channel, message, (originalMessage ,id) -> {
                StompFrame newMessage = new StompFrame("MESSAGE", originalMessage.getBody());
                newMessage.addHeader("id", Integer.toString(id));
                return newMessage;
            }, connectionId);
    
            if(result == -1){
                StompFrame errorFrame = new StompFrame("ERROR", "The Client is not subscribed to the given channel");
                errorFrame.addHeader("message", "Cannot send message");
                connections.send(this.connectionId, errorFrame);
            }
        }
        
    }

   
    public boolean shouldTerminate() {
        return shouldTerminate;

    }
}
