package network_lab2_DV;


//·�ɱ���
public class RouterTable{
	public String DestNode;		//Ŀ��ڵ�
	public double Distance;		//����
	public String Neighbor;		//��һ��
	
	public void myclone(RouterTable r) {
		DestNode = r.DestNode;
		Distance = r.Distance;
		Neighbor = r.Neighbor;
	}
}