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

	public enum Direction {
		Horizontal, Vertical
	};

	public static Direction direction = Direction.Horizontal;

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
		Variables allVars = blocks.getAllVariables();
		if (direction == Direction.Horizontal) {
			hRender(g, allVars);
		} else {
			vRender(g, allVars);
		}
	}

	private void hRender(Graphics2D g, Variables allVars) {
		posLookup.clear();
		int y = 0, ystep = getSize().height / (allVars.size());
		float h = 0, hstep = 1f / 7f;
		RectangleView firstRect = (RectangleView) allVars.min().data
				.get(RectangleView.class);
		RectangleView lastRect = (RectangleView) allVars.max().data
				.get(RectangleView.class);
		double xMin = firstRect.x, xMax = lastRect.getMaxX();
		double xRange = xMax - xMin;
		double xScale = getSize().width / xRange;
		int vcounter = 0;
		for (Block b : blocks) {
			Color c = RectangleView.defaultRectColour;
			if (b.variables.size() > 1) {
				Color hsb = Color.getHSBColor(h, 0.5f, 1f);
				c = new Color(hsb.getRed(), hsb.getGreen(), hsb.getBlue(), 200);
				h += hstep;
			}
			for (Variable v : b.variables) {
				RectangleView rv = (RectangleView) v.data
						.get(RectangleView.class);
				int w = (int) (rv.width * xScale);
				int x = (int) ((rv.x - xMin) * xScale);
				// assert(x+w<getSize().width);
				posLookup.put(v, y);
				rv.colour = c;
				rv.draw(g, x, y, w, ystep);
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
			g.setPaint(new Color(c.colour.getRed(), c.colour.getGreen(),
					c.colour.getBlue(), 100));
			drawArrow(g, xl, posLookup.get(l), xr, posLookup.get(r), 5);
		}
	}

	private void vRender(Graphics2D g, Variables allVars) {
		posLookup.clear();
		int x = 0, xstep = getSize().width / (allVars.size());
		float hue = 0, hstep = 1f / 7f;
		RectangleView firstRect = (RectangleView) allVars.min().data
				.get(RectangleView.class);
		RectangleView lastRect = (RectangleView) allVars.max().data
				.get(RectangleView.class);
		double yMin = firstRect.y, yMax = lastRect.getMaxY();
		double yRange = yMax - yMin;
		double yScale = getSize().height / yRange;
		int vcounter = 0;
		for (Block b : blocks) {
			Color c = RectangleView.defaultRectColour;
			if (b.variables.size() > 1) {
				Color hsb = Color.getHSBColor(hue, 0.5f, 1f);
				c = new Color(hsb.getRed(), hsb.getGreen(), hsb.getBlue(), 200);
				hue += hstep;
			}
			for (Variable v : b.variables) {
				RectangleView rv = (RectangleView) v.data
						.get(RectangleView.class);
				int h = (int) (rv.height * yScale);
				int y = (int) ((rv.y - yMin) * yScale);
				// assert(x+w<getSize().width);
				posLookup.put(v, x);
				rv.colour = c;
				rv.draw(g, x, y, xstep, h);
				x += xstep;
				vcounter++;
			}
		}
		for (Constraint c : constraints) {
			Variable l = c.left, r = c.right;
			int yl = (int) ((l.getPosition() - yMin) * yScale);
			int yr = (int) ((r.getPosition() - yMin) * yScale);
			c.colour = Color.BLUE;
			if (c.isViolated()) {
				c.colour = Color.RED;
			} else if (c.isTight()) {
				c.colour = Color.GREEN;
			}
			g.setPaint(new Color(c.colour.getRed(), c.colour.getGreen(),
					c.colour.getBlue(), 100));
			drawArrow(g, posLookup.get(l), yl, posLookup.get(r), yr, 5);
		}
	}

	/**
	 * Draws an arrow head from (x,y) to (X,Y). The arrow has a triangular head
	 * of size given by the parameter
	 * 
	 * @param g
	 * @param x
	 *            start point is (x,y)
	 * @param y
	 *            start point is (x,y)
	 * @param X
	 *            end point is (X,Y)
	 * @param Y
	 *            end point is (X,Y)
	 * @param size
	 *            of arrow base in pixels
	 */
	private void drawArrow(Graphics2D g, int x, int y, int X, int Y, int size) {
		g.drawLine(x, y, X, Y);
		double s = (double) size;
		double dx = X - x;
		double dy = Y - y;
		double len = Math.sqrt((double) (dx * dx + dy * dy));
		// u is unit vector from (x,y) to (X,Y)
		double ux = dx / len;
		double uy = dy / len;
		// a is beginning of arrow triangle
		double ax = X - s * ux;
		double ay = Y - s * uy;
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

	Hashtable<Variable, Integer> posLookup = new Hashtable<Variable, Integer>();

}
