package bit.minisys.minicc.semantic;
import java.util.LinkedList;
import java.util.List;

public class ErrorHandler {
	public List lis = new LinkedList<>();
	
	//
	public void addES01(String name) {
		String error = "ES01 > ";
		error = error + "Identifier ["+ name + "] is not defined";
		
		this.lis.add(error);
	}
	
	//δ����ʹ��
	public void addES02(String var) {
		String error = "ES02 > ";
		error = error + "Declaration ["+var+ "] is defined";
		
		this.lis.add(error);
	}
	
	//break
	public void addES03() {
		String error = "ES03 > ";
		error = error + "breakstatement must be in a loopstatement";
		
		this.lis.add(error);
	}
	
	//��������������ƥ��
	public void addES04(String func) {
		String error = "ES04 > ";
		error = error + "FunctionCall [" +func + "]'s args num isn't matched";
		
		this.lis.add(error);
	}	
	
	//�����������Ͳ�ƥ��
	public void addES04_1(String func) {
		String error = "ES04 > ";
		error = error + "FunctionCall [" +func + "]'s args type isn't matched";
		
		this.lis.add(error);
	}	
	
	//�������Խ��
	public void addES06(String arr) {
		String error = "ES06 > ";
		error = error + "array [" + arr + "] access out of bound";
		
		this.lis.add(error);
	}	
	
	//����δ��return����
	public void addES07(String label) {
		String error = "ES07 > ";
		error = error + "label [" +label + "] is not defined";
		
		this.lis.add(error);
	}	

	//����δ��return����
	public void addES08(String func) {
		String error = "ES08 > ";
		error = error + "function [" +func + "] lack of return";
		
		this.lis.add(error);
	}	
	
	//ȫ�ֶ�������
	public void addES09() {
		String error = "ES09 > ";
		error = error + "program's items should be Declaration or FunctionDefine";
		
		this.lis.add(error);
	}
	
	public void addES02_1(String func) {
		String error = "ES02 > ";
		error = error + "Function [" + func +"] is defined";
		
		this.lis.add(error);
	}
	
	public void addES01_1(String func) {
		if(func.equals("Mars_PrintStr")||
		   func.equals("Mars_GetInt")||
		   func.equals("Mars_PrintInt")) return;
		String error = "ES01 > ";
		error = error + "FunctionCall [" + func +"] is not declarated";
		
		this.lis.add(error);
	}
	
	//break
	public void addES12() {
		String error = "ES12 > ";
		error = error + "continue not in loop";
		
		this.lis.add(error);
	}
	
	//break
	public void addES13(String func) {
		String error = "ES13 > ";
		error = error + "function [" +func + "] is void with return";
		
		this.lis.add(error);
	}	
	
	//����ά�������������
	public void addES14(String name) {
		String error = "ES14 > ";
		error = error + "array [" + name + "] dim should be integer";
		
		this.lis.add(error);
	}	
	//�Ƿ���������
	public void addES15() {
		String error = "ES15 > ";
		error = error + "function call illegal";
		
		this.lis.add(error);
	}		
	
	//��ǩ�ظ�
	public void addES16(String name) {
		String error = "ES10 > ";
		error = error + "label [" + name + "] is defined";
		
		this.lis.add(error);
	}		
	
	public void addES17(String func) {
		String error = "ES11 > ";
		error = error + "Function [" + func +"] is not defined global";
		
		this.lis.add(error);
	}	
	public void output() {
		if(lis.size()==0) return;
		System.out.println("Error:");
		for(int i=0;i<this.lis.size();i++) {
			System.out.println((String)this.lis.get(i));
		}
	}
	 
}
