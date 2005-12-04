package placement;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Rectangle2D;
import java.awt.image.VolatileImage;
import java.util.Hashtable;

import javax.swing.JPanel;

public class DebugPanel extends JPanel {
	private Constraints constraints;

	private Blocks blocks;

	public DebugPanel(Blocks blocks, Constraints constraints) {
		super(true);
		this.blocks = blocks;
		this.constraints = constraints;
	}

	private VolatileImage volatileImg;

	public void paintComponent(Graphics g) {
		// create the hardware accelerated image.
		createBackBuffer();

		// Main rendering loop. Volatile images may lose their contents.
		// This loop will continually render to (and produce if neccessary)
		// volatile images
		// until the rendering was completed successfully.
		do {

			// Validate the volatile image for the graphics configuration of
			// this
			// component. If the volatile image doesn't apply for this graphics
			// configuration
			// (in other words, the hardware acceleration doesn't apply for the
			// new device)
			// then we need to re-create it.
			GraphicsConfiguration gc = this.getGraphicsConfiguration();
			int valCode = volatileImg.validate(gc);

			// This means the device doesn't match up to this hardware
			// accelerated image.
			if (valCode == VolatileImage.IMAGE_INCOMPATIBLE) {
				createBackBuffer(); // recreate the hardware accelerated image.
			}

			Graphics offscreenGraphics = volatileImg.getGraphics();
			render(offscreenGraphics); // call core paint method.

			// paint back buffer to main graphics
			g.drawImage(volatileImg, 0, 0, this);
			// Test if content is lost
		} while (volatileImg.contentsLost());
	}

	// This method produces a new volatile image.
	private void createBackBuffer() {
		GraphicsConfiguration gc = getGraphicsConfiguration();
		volatileImg = gc.createCompatibleVolatileImage(getWidth(), getHeight());
	}

	public void render(Graphics gOld) {
		// super.paintComponent(gOld);
		Graphics2D g = (Graphics2D) gOld;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		Color original = g.getColor();
		g.setColor(getBackground());
		Dimension size = getSize();
		g.fillRect(0, 0, size.width, size.height);
		g.setColor(original);
		super.paintChildren(g);
		yLookup.clear();
		Variables allVars = blocks.getAllVariables();
		int y = 0, ystep = getSize().height / (allVars.size());
		float h = 0, hstep = 1f / 7f;
		double xMin = allVars.getMinPos(), xMax = allVars.getMaxPos();
		double xRange = xMax - xMin;
		double xScale = (getSize().width - 20) / xRange;
		int vcounter = 0;
		for (Block b : blocks) {
			Color c = Color.WHITE;
			if (b.variables.size() > 1) {
				c = Color.getHSBColor(h, 0.5f, 1f);
				h += hstep;
			}
			for (Variable v : b.variables) {
				int x = (int) ((v.getPosition() - xMin) * xScale);
				int w = 20;
				try {
					w = (int) (((Rectangle2D) v.data.get(Rectangle2D.class))
							.getWidth() * xScale);
				} catch (NullPointerException e) {
					// variable is not associated with a Rectangle
				}
				Rectangle r = new Rectangle(x, y, w, ystep);
				yLookup.put(v, y);
				v.colour = c;
				g.setPaint(c);
				g.fill(r);
				g.setPaint(Color.BLACK);
				g.draw(r);
				g.drawString(v.name, x, y + 10);
				y += ystep;
				vcounter++;
			}
		}
		for (Constraint c : constraints) {
			Variable l = c.left, r = c.right;
			int xl = (int) ((l.getPosition() - xMin) * xScale);
			int xr = (int) ((r.getPosition() - xMin) * xScale);
			c.colour = Color.BLUE;
			if (c.isViolated()) {
				c.colour = Color.RED;
			} else if (c.isTight()) {
				c.colour = Color.GREEN;
			}
			g.setPaint(c.colour);
			g.drawLine(xl, yLookup.get(l), xr, yLookup.get(r));
			g.drawOval(xr - 5, yLookup.get(r) - 5, 10, 10);
		}
	}

	public void updateDrawing() {
		paintComponent(getGraphics());
	}

	Hashtable<Variable, Integer> yLookup = new Hashtable<Variable, Integer>();

}
