import java.util.Iterator;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class ConnectionsImpl<T> implements Connections<T>{

    private static volatile ConnectionsImpl<?> instance;
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> connectionsMap;             // ID to Connection Handler
    private ConcurrentHashMap<String, ConcurrentLinkedQueue<Integer[]>> channelsMap;    // Channel name to List of ID's
    private ConcurrentHashMap<String, String> loginMap;                                 // username to password
    private final ReentrantReadWriteLock lock;
    final AtomicInteger connectionIDGenerator;                                          // Generate ID for new connections
    

    public static <T> ConnectionsImpl<T> getInstance(){
        if (instance == null){
            synchronized(ConnectionsImpl.class){
                if (instance == null){
                    instance = new ConnectionsImpl<>();
                }
            }
        }
        return (ConnectionsImpl<T>) instance;
    }

    private ConnectionsImpl(){
        this.connectionsMap = new ConcurrentHashMap<>();
        this.channelsMap = new ConcurrentHashMap<>();
        this.loginMap = new ConcurrentHashMap<>();
        this.connectionIDGenerator = new AtomicInteger(0);
        this.lock = new ReentrantReadWriteLock();
    }


    // Send the message to the connection with the specific connection ID
    public boolean send(int connectionId, T message){
        if(connectionsMap.get(connectionId) != null){
            lock.readLock().lock();
            try{
                if(connectionsMap.get(connectionId) != null){
                    connectionsMap.get(connectionId).send(message);
                    return true;
                }
            }
            finally{
                lock.readLock().unlock();       
            }
            
        }

        return false;
    }

    // Send the message for each active connection that subscribed to this specific channel
    public void send(String channel, T message){
        lock.readLock().lock();
        try{
            for(Integer[] ID : channelsMap.get(channel)){
                if (connectionsMap.get(ID[0]) != null){
                    connectionsMap.get(ID[0]).send(message);
                }
            } 
        } finally{
            lock.readLock().unlock();  
        }
    }
    
    // Remove an active connection from the connectionMap
    public void disconnect(int connectionId){
        connectionsMap.remove(connectionId);
        unsubscribeChannel(connectionId);
    } 

    private void unsubscribeChannel(int connectionId){
        lock.writeLock().lock();
        try{
            for (String channel : channelsMap.keySet()){
                Iterator<Integer[]> iter = channelsMap.get(channel).iterator();
                while(iter.hasNext()){
                    Integer[] pair = iter.next();
                    if(pair[0] == connectionId){
                        iter.remove();
                    }
                }
            }
        } finally{
            lock.writeLock().unlock();
        }
    }

    public int login(ConnectionHandler<T> handler, String userName, String password){
        synchronized(loginMap){
            // check if user doesn't exist
            if(loginMap.get(userName) == null){ 
                loginMap.put(userName, password);          
            }
            // check if user password is correct
            if(loginMap.get(userName).equals(password)){       
                connectionsMap.put(connectionIDGenerator.get(), handler);
                return connectionIDGenerator.incrementAndGet() - 1;
            }
    
            else{
                    // return ERROR (incorrect password)
            }
        }
        return -1; // if login wasn't successful
    }
}

