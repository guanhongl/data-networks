/*
 *
 * NAME:        Guanhong Li
 *
 * HOMEWORK:    1
 *
 * CLASS:       ICS 451
 *
 * INSTRUCTOR:  Ravi Narayan
 *
 * DATE:        September 8, 2021
 *
 * FILE:        MyServer.java
 *
 * DESCRIPTION: This file contains the server for homework 1.
 *
 */
import java.net.ServerSocket;
import java.net.Socket;
import java.io.IOException;
import java.io.PrintStream;
import java.io.BufferedReader;
import java.io.FileReader;

public class MyServer {
  
    /**
     * Starts a simple server which sends a text file upon client connection.
     * @param args the command line arguments
     * @throws IOException
     */
    public static void main(String[] args) throws IOException {
        int portNumber = Integer.parseInt(args[0]);
        String fileLine;
        
        PrintStream serverLog = new PrintStream("server_log.txt");
        
        serverLog.println("Starting connection...");
        ServerSocket serverSocket = new ServerSocket(portNumber);
        serverLog.println("Connection started at port " + portNumber + ".");
        
        serverLog.println("Listening for client connection...");
        Socket clientSocket = serverSocket.accept();
        serverLog.println("Client connected.");
        
        serverLog.println("Sending message to client...");
        PrintStream clientOutput = new PrintStream(clientSocket.getOutputStream(), true);
        BufferedReader fileInput = new BufferedReader(new FileReader("server_file.txt"));
        while ((fileLine = fileInput.readLine()) != null) {
            clientOutput.println(fileLine);
        }
        serverLog.println("Message sent to client.");
        
        fileInput.close();
        clientOutput.close();
        
        serverLog.println("Closing connection...");
        clientSocket.close();
        serverSocket.close();
    }
}