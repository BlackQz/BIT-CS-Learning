package network_lab2_DV;

import java.util.LinkedList;
import java.util.Queue;
import java.util.Scanner;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileOutputStream;

import javax.swing.JFrame;
import javax.swing.KeyStroke;

enum EventType{
	time_out,
	receive_message;
}

public class Main{
	static String IP="127.0.0.1";
	static int maxRouterNum = 10;
	static int Frequency;	//���ͼ��
	static double Unreachable;	//���ɴ����
	static int MaxValidTime;	//���ȴ�ʱ��
	static Router router;
	static Sender sender;
	static Receiver receiver;
	static public Queue<EventType> event = new LinkedList<EventType>();
	static public Queue<String> recv = new LinkedList<String>();
	
	static public boolean pause = false;
	static Control control;
	
	static File sendlog;
	static File recvlog;
	static int sequence = 0;

	
	public static void main(String[] args) throws Exception {
		//����У��
		//arg1: �ڵ�id
		//arg2: UDP�˿ں�
		//arg3: �ڵ��ʼ���ļ���
		//
		Scanner scanner = new Scanner(new File("config.txt"));
		while(scanner.hasNext()) {
			String type = scanner.next();
			if (type.contains("Frequency")) {
				Frequency = scanner.nextInt();
			}
			if (type.contains("Unreachable")) {
				Unreachable = scanner.nextDouble();
			}
			if (type.contains("MaxValidTime")) {
				MaxValidTime =  scanner.nextInt();
			}
		}
		
		if (args.length != 3) {
			System.out.println("Wrong Parameter");
			return;
		}
		else {
			File menu = new File("log");
			if (!menu.exists()) {
				menu.mkdir();
			}
			String sendfile = "log/" + args[0] + "_send.txt";
			String recvfile=  "log/" + args[0] + "_recv.txt";
			sendlog = new File(sendfile);
			recvlog = new File(recvfile);
			if (!sendlog.exists()) {
				sendlog.createNewFile();
			}
			if (!recvlog.exists()) {
				recvlog.createNewFile();
			}
			router = new Router(args[0], Integer.valueOf(args[1]), args[2]);
			sender = new Sender(args[2], IP);
			sender.start();
			receiver = new Receiver(Integer.valueOf(args[1]), IP);
			receiver.start();
			control = new Control();
			control.start();
		}
		while(true) {
			EventType e = event.poll();
			String str = recv.poll();
			if (str == null) {
				Thread.sleep(100);
				continue;
			}
			if (e == EventType.receive_message) {
				String[] item = str.split(" ");
				int length = Integer.valueOf(item[0]);
				if (length == 0) {
					continue;
				}
				sequence++;
				Message[] m = new Message[length];
				for (int i = 0; i < length; i++) {
					m[i] = new Message();
					m[i].SrcNode = item[i * 4 + 1];
					m[i].DestNode = item[i * 4 + 2];
					m[i].NeighborNode = item[i * 4 + 3];
					m[i].Distance = Double.valueOf(item[i * 4 + 4]);
				}
				
				FileOutputStream out = new FileOutputStream(Main.recvlog,true); 
				out.write(("## Received. Source Node = " + m[0].SrcNode + ";").getBytes());
				out.write((" Sequence Number = " + sequence + "\n").getBytes());
				for (int i = 0; i < length; i++) {
					out.write(("DestNode = " + m[i].DestNode + "; ").getBytes());
					out.write(("Distance = " + m[i].Distance + "; ").getBytes());
					out.write(("Neighbor = " + m[i].NeighborNode + "\n").getBytes());
				}	
				out.write("\n".getBytes());
				out.close();
				
				router.update1(m, length);
			}
			if (e == EventType.time_out) {
				router.remove(str);
			}
		}
	}
}
