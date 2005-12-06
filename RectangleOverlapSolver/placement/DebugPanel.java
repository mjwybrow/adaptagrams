package placement;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.GeneralPath;
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
				Color hsb = Color.getHSBColor(h, 0.5f, 1f);
				c=new Color(hsb.getRed(),hsb.getGreen(),hsb.getBlue(),200);
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
				RectangleDrawerPanel.drawStringInRectangle(this, g, r, v.name);
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
			g.setPaint(new Color(c.colour.getRed(),c.colour.getGreen(),c.colour.getBlue(),160));
			drawArrow(g, xl, yLookup.get(l), xr, yLookup.get(r), 5);
		}
	}

	/**
	 * Draws an arrow head from (x,y) to (X,Y).
	 * The arrow has a triangular head of size given by the parameter
	 * 
	 * @param g
	 * @param x start point is (x,y)
	 * @param y start point is (x,y)
	 * @param X end point is (X,Y)
	 * @param Y end point is (X,Y)
	 * @param size of arrow base in pixels
	 */
	private void drawArrow(Graphics2D g, int x, int y, int X, int Y, int size) {
		g.drawLine(x,y,X,Y);
		double s=(double)size;
		double dx=X-x;
		double dy=Y-y;
		double len=Math.sqrt((double)(dx*dx+dy*dy));
		// u is unit vector from (x,y) to (X,Y)
		double ux=dx/len;
		double uy=dy/len;
		// a is beginning of arrow triangle
		double ax=X-s*ux;
		double ay=Y-s*uy;
		// v is vector from start to end of arrow triangle
		double vx = X - ax;
		double vy = Y - ay;
		// w is perpendicular to v and half the length
		double wx = -vy / 2;
		double wy = vx / 2;
		// a and b are corners of arrow head
		Point p1 = new Point((int) (ax - wx), (int) (ay - wy));
		Point p2 = new Point((int) (ax + wx), (int) (ay + wy));
		GeneralPath path = new GeneralPath(GeneralPath.WIND_EVEN_ODD, 5);
		path.moveTo(p1.x, p1.y);
		path.lineTo(p2.x, p2.y);
		path.lineTo(X, Y);
		path.lineTo(p1.x, p1.y);
		path.closePath();
		g.fill(path);
	}

	public void updateDrawing() {
		paintComponent(getGraphics());
	}

	Hashtable<Variable, Integer> yLookup = new Hashtable<Variable, Integer>();

}
