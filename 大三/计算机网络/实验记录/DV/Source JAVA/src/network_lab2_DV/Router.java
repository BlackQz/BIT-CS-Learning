package network_lab2_DV;

import java.io.File;
import java.util.Scanner;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.LinkedList;

public class Router {
	public String localIndex;													//�ڵ�ID
	public int localPort;														//�ڵ�˿�
	public TreeMap<String, Double> neighborDistance = new TreeMap<>();			//�ھӽڵ����
	public TreeMap<String, Timer> timer = new TreeMap<>();					
	public LinkedList<RouterTable> routerTable = new LinkedList<RouterTable>();
	public int tableSize;														//·�ɱ�

	public String ExtendInfo(String s,String t,String n) {
		return s+" "+t+" "+n;
	};
	HashMap<String,Double>Info = new HashMap<>();	//��¼�������ΪS���յ�ΪT����һ��ΪN�����·
	public double get(String s,String t,String n) {
		if (!Info.containsKey(ExtendInfo(s,t,n))) {
			Info.put(ExtendInfo(s,t,n), Main.Unreachable);
		}
		return Info.get(ExtendInfo(s,t,n));
	}
	public void set(String s,String t,String n,double value) {
		Info.put(ExtendInfo(s,t,n), value);
	}
	
	public void update() {	//����Info�õ��µ�·�ɱ�
		TreeMap<String,Double> dis = new TreeMap<>();
		TreeMap<String,String> nei = new TreeMap<>();
		for (String s : Info.keySet()) {
			String T=s.split(" ")[1];
			String N=s.split(" ")[2];
			if (!dis.containsKey(T) || dis.get(T) > Info.get(s) ) {
				dis.put(T, Info.get(s));
				nei.put(T, N);
			}
		}
		routerTable = new LinkedList<RouterTable>();
		tableSize = 0;
		for (String item : dis.keySet()) {
			RouterTable tmp = new RouterTable();
			tmp.DestNode = item;
			tmp.Distance = dis.get(item);
			tmp.Neighbor = nei.get(item);
			if (!tmp.DestNode.equals(localIndex) && tmp.Distance < Main.Unreachable) {
				routerTable.add(tmp);
				tableSize++;
			}
		}
	}
	
	//��ʼ�� 
	public Router(String _localIndex, int _localPort, String filename) throws Exception{
		localIndex = _localIndex;
		localPort = _localPort;
		File file = new File(filename);
		Scanner scan = new Scanner(file);
		tableSize = 0;
		while(scan.hasNext()) {
			String neighbor = scan.next();
			double distance = scan.nextDouble();
			scan.next();
			timer.put(neighbor, new Timer(neighbor));
			timer.get(neighbor).start();
			set(localIndex,neighbor,neighbor, distance);
		}
		scan.close();
		update();
	}
	

	
	//��·�ɱ���ɾ����һ����Neighbor��������
	public void remove(String Neighbor) {
		for (String s : Info.keySet()) {
			if (s.split(" ")[2].equals(Neighbor)) {
				Info.put(s, Main.Unreachable);
			}
		}
		update();
	}
	
	
	//���ھӷ�������Ϣ����·�ɱ�
	public void update1(Message m[], int length) {
		timer.get(m[0].SrcNode).receive();
		for (int i=0;i<length;i++) {
			double newDis = get(localIndex,m[i].SrcNode,m[i].SrcNode) + m[i].Distance;
			set(localIndex,m[i].DestNode,m[i].SrcNode,newDis);
		}
		update();
	}
}
