package placement;

import javax.swing.JFrame;

public class DebugFrame extends JFrame {
	private Constraints constraints;

	private Blocks blocks;

	private DebugPanel panel;

	DebugFrame(Blocks blocks, Constraints constraints) {
		super();
		this.blocks = blocks;
		this.constraints = constraints;
		if (blocks.isEmpty()) {
			this.dispose();
			return;
		}
		panel = new DebugPanel(blocks, constraints);
		panel.setSize(324, 700);
		setSize(324, 700);
		setLocation(700, 0);
		add(panel);
		setVisible(true);
	}

	void animate() {
		if (panel != null) {
			panel.updateDrawing();
		}
	}
}
