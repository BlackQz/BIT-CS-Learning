package bit.minisys.minicc.ncgen;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.LinkedList;

class SymbolMap{
	Map<String,Sunit> sym2svar = new HashMap<>();
	Map<Sunit,Integer> svar2off = new HashMap<>();
	
	SymbolMap(){
		sym2svar = new HashMap<>();
		svar2off = new HashMap<>();
	}
}

class Sunit{
	String symbol;
	int index;
	boolean inreg;		//λ�ڼĴ����У���ζ�ź��ڴ��б������ڲ�һ����
	Reg areg;
	Sunit(String s){
		symbol = s;
		index = -1;
		inreg = false;
	}
	Sunit(String s,int i)
	{
		symbol = s;
		index = i;
		inreg = false;
	}
	void savetomem() {
		areg.var = null;
		areg = null;
		inreg = false;
	}
}