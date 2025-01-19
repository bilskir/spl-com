import java.util.Iterator;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class ConnectionsImpl<T> implements Connections<T>{

    private static ConnectionsImpl<?> instance = null;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> connectionsMap;             // ID to Connection Handler
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer[]>> channelsMap;    // Channel name to List of ID's
    private ConcurrentHashMap<String, String> loginMap;                                 // username to password
    final AtomicInteger connectionIDGenerator;                                                     // Generate ID for new connections


    public static <T> ConnectionsImpl<T> getInstance(){
        if(instance == null){
            instance = (ConnectionsImpl<T>) new ConnectionsImpl();
        }

        return (ConnectionsImpl<T>) instance;
    }


    private ConnectionsImpl(){
        this.connectionsMap = new ConcurrentHashMap<>();
        this.channelsMap = new ConcurrentHashMap<>();
        this.loginMap = new ConcurrentHashMap<>();
        this.connectionIDGenerator.set(0);
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
        for(Integer[] ID : channelsMap.get(channel)){
            if (connectionsMap.get(ID[0]) != null){
                connectionsMap.get(ID[0]).send(message);
            }
        }    
    }
    
    // Remove an active connection from the connectionMap
    public void disconnect(int connectionId){
        connectionsMap.remove(connectionId);
        unsubscribeChannel(connectionId);
    } 

    private void unsubscribeChannel(int connectionId){
        for (String channel : channelsMap.keySet()){
            Iterator<Integer[]> iter = channelsMap.get(channel).iterator();
            while(iter.hasNext()){
                Integer[] pair = iter.next();
                if(pair[0] == connectionId){
                    iter.remove();
                }
            }
        }
    }

    public int login(ConnectionHandler<T> handler, String userName, String password){
        if(loginMap.get(userName) == null){
            loginMap.put(userName, password);
        }

        else{
            if(loginMap.get(userName).equals(password)){
                connectionsMap.put(connectionIDGenerator.get(), handler);
                connectionIDGenerator.incrementAndGet();
            }

            else{
                // return ERROR (incorrect password)
            }
        }

        return connectionIDGenerator.get() - 1;
    }

}

