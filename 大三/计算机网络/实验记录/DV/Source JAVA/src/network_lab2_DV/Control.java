package network_lab2_DV;

import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.net.DatagramPacket;

import javax.swing.JFrame;
import javax.swing.KeyStroke;


//ѭ��������Ϣ���߳���
public class Control extends Thread implements KeyListener {
    @Override
    public void keyPressed(KeyEvent e) {
        //����ĳ����ʱ���ô˷���
    	if (e.getKeyCode() == KeyEvent.VK_P) {
			Main.pause = true;
		}
    	if (e.getKeyCode() == KeyEvent.VK_S) {
    		Main.pause = false;
		}
    	if (e.getKeyCode() == KeyEvent.VK_K) {
    		System.exit(0);
		}    	
    }
 
    @Override
    public void keyReleased(KeyEvent e) {
        //�ͷ�ĳ����ʱ���ô˷���
    }
 
    @Override
    public void keyTyped(KeyEvent e) {
        //����ĳ����ʱ���ô˷���
    }

    public void run() {
		JFrame jf = new JFrame("MyListener");
		jf.addKeyListener(new Control());
		jf.setBounds(300,300,800,600);
		jf.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		jf.setVisible(true);
	}
}
