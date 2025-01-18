import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T>{

    private static ConnectionsImpl<?> instance = null;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> connectionsMap;   
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer,ConnectionHandler<T>>> channelsMap;

    public static <T> ConnectionsImpl<T> getInstance(){
        if(instance == null){
            instance = (ConnectionsImpl<T>) new ConnectionsImpl();
        }

        return (ConnectionsImpl<T>) instance;
    }


    private ConnectionsImpl(){
        this.connectionsMap = new ConcurrentHashMap<>();
        this.channelsMap = new ConcurrentHashMap<>();
    }
    


    // Send the message to the connection with the specific connection ID
    public boolean send(int connectionId, T message){
        if(connectionsMap.get(connectionId) != null){
            connectionsMap.get(connectionId).send(message);
            return true;
        }

        return false;
    }

    // Send the message for each active connection that subscribed to this specific channel
    public void send(String channel, T message){
        for(ConnectionHandler<T> handler : channelsMap.get(channel).values()){
            if (connectionsMap.get(handler) != null){
                handler.send(message);
            }
        }    
    }
    
    // Remove an active connection from the connectionMap
    public void disconnect(int connectionId){
        connectionsMap.remove(connectionId);
    } 
}

