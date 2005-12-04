package placement;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.event.MouseEvent;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.image.VolatileImage;
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Observable;
import java.util.Observer;
import java.util.Random;

import javax.imageio.ImageIO;
import javax.swing.JPanel;
import javax.swing.event.MouseInputListener;
import javax.swing.filechooser.FileFilter;

class BlocksFileFilter extends FileFilter {
	public File lastSelectedFile = null;

	public boolean accept(File file) {
		return file.isDirectory() || file.getName().endsWith(".blocks")
				|| file.getName().endsWith(".dot");
	}

	public String getDescription() {
		return "Blocks and Dot graph files";
	}
}

public class RectangleDrawerPanel extends JPanel implements Printable,
		MouseInputListener, Observer {
	private int prevX, prevY;

	private boolean dragging;

	private Graphics2D g;

	private Rectangle2D rect;

	private ArrayList<Rectangle2D> rectangles = new ArrayList<Rectangle2D>();

	private Graph graph = null;

	private ArrayList<Rectangle2D> undoRectangles = new ArrayList<Rectangle2D>();;

	private Constraints constraints;

	BlocksFileFilter fileFilter = new BlocksFileFilter();

	protected Hashtable<Rectangle2D, Color> rectangleColourMap = new Hashtable<Rectangle2D, Color>();

	protected void generateRandom() {
		clear();
		Dimension dim = getSize();
		int n = 50;
		Random rand = new Random();
		double w = dim.width / 3.0;
		double h = dim.height / 3.0;
		for (int i = 0; i < n; i++) {
			Rectangle2D r = new Rectangle2D.Double(w + rand.nextDouble() * w, h
					+ rand.nextDouble() * h, rand.nextDouble() * (w / 3.0),
					rand.nextDouble() * (h / 3.0));
			rectangles.add(r);
		}
		int overlapCount = 0;
		for (int i = 0; i < rectangles.size(); i++) {
			Rectangle2D u = rectangles.get(i);
			for (int j = i + 1; j < rectangles.size(); j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.intersects(v))
					overlapCount++;
			}
		}
		System.out.println("Random graph has " + overlapCount + " overlaps.");
		fitToScreen();
		repaint();
	}

	void fitToScreen() {
		ArrayList<Rectangle2D> rectangles = getRectangles();
		double xmax = 0;
		double xmin = Double.MAX_VALUE;
		double ymax = 0;
		double ymin = Double.MAX_VALUE;
		for (Rectangle2D r : rectangles) {
			xmin = Math.min(xmin, r.getMinX());
			xmax = Math.max(xmax, (r.getMinX() + r.getWidth()));
			ymin = Math.min(ymin, r.getMinY());
			ymax = Math.max(ymax, (r.getMinY() + r.getHeight()));
		}
		double currentWidth = xmax - xmin;
		double currentHeight = ymax - ymin;
		double targetWidth = getSize().width;
		double targetHeight = getSize().height;
		double xscale = targetWidth / currentWidth;
		double yscale = targetHeight / currentHeight;
		double scale = 1;
		if (xscale < yscale && xscale * currentHeight <= targetHeight) {
			scale = xscale;
		} else {
			scale = yscale;
		}
		for (Rectangle2D r : rectangles) {
			double x = r.getMinX();
			double y = r.getMinY();
			double w = r.getWidth();
			double h = r.getHeight();
			x -= xmin < 0 ? xmin : 0;
			y -= ymin < 0 ? ymin : 0;
			if (scale < 1) {
				x *= scale;
				w *= scale;
				y *= yscale;
				h *= yscale;
			}
			r.setRect(Math.floor(x), Math.floor(y), Math.floor(w), Math
					.floor(h));
		}
	}

	protected void load(File f) {
		clear();
		ObjectInput input = null;
		if (f.getPath().endsWith(".dot")) {
			GraphParser g = new GraphParser(f.getPath());
			graph = g.getGraph();
			fileFilter.lastSelectedFile = f;
		} else {
			try {
				// use buffering
				InputStream file = new FileInputStream(f);
				InputStream buffer = new BufferedInputStream(file);
				input = new ObjectInputStream(buffer);
				// deserialize the List
				rectangles = (ArrayList<Rectangle2D>) input.readObject();
			} catch (Exception ex) {
				ex.printStackTrace();
			} finally {
				try {
					if (input != null) {
						// close "input" and its underlying streams
						input.close();
						fileFilter.lastSelectedFile = f;
					}
				} catch (IOException ex) {
					ex.printStackTrace();
				}
			}
		}
		fitToScreen();
		repaint();
	}

	/**
	 * 
	 */
	protected void backup() {
		undoRectangles = new ArrayList<Rectangle2D>();
		for (Rectangle2D r : getRectangles()) {
			Rectangle2D nr = new Rectangle2D.Double();
			nr.setRect(r.getMinX(), r.getMinY(), r.getWidth(), r.getHeight());
			undoRectangles.add(nr);
		}
	}

	protected void undo() {
		rectangles = new ArrayList<Rectangle2D>(undoRectangles);
	}

	/**
	 * @return
	 */
	protected BlocksFileFilter getFileFilter() {
		return fileFilter;
	}

	/**
	 * 
	 */
	public RectangleDrawerPanel() {
		super(true);
		addMouseListener(this);
		addMouseMotionListener(this);
	}

	public void clear() {
		rectangles = new ArrayList<Rectangle2D>();
		constraints = null;
		graph = null;
		repaint();
	}

	private VolatileImage volatileImg;

	public void paintComponent(Graphics g) {
		// create the hardware accelerated image.
		createBackBuffer();

		// Main rendering loop. Volatile images may lose their contents. 
		// This loop will continually render to (and produce if neccessary) volatile images
		// until the rendering was completed successfully.
		do {

			// Validate the volatile image for the graphics configuration of this 
			// component. If the volatile image doesn't apply for this graphics configuration 
			// (in other words, the hardware acceleration doesn't apply for the new device)
			// then we need to re-create it.
			GraphicsConfiguration gc = this.getGraphicsConfiguration();
			int valCode = volatileImg.validate(gc);

			// This means the device doesn't match up to this hardware accelerated image.
			if (valCode == VolatileImage.IMAGE_INCOMPATIBLE) {
				createBackBuffer(); // recreate the hardware accelerated image.
			}

			Graphics2D offscreenGraphics = (Graphics2D) volatileImg
					.getGraphics();
			render(getWidth(), getHeight(), offscreenGraphics); // call core paint method.

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

	public void render(int width, int height, Graphics2D g) {
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		Color original = g.getColor();
		g.setColor(getBackground());
		g.fillRect(0, 0, width, height);
		g.setColor(original);
		super.paintChildren(g);
		if (graph != null) {
			for (Line2D l : graph.getLines()) {
				int x1 = (int) l.getP1().getX();
				int y1 = (int) l.getP1().getY();
				int x2 = (int) l.getP2().getX();
				int y2 = (int) l.getP2().getY();
				g.drawLine(x1, y1, x2, y2);
			}
		}
		for (Rectangle2D r : getRectangles()) {
			Color c = new Color(228, 228, 205);
			if (rectangleColourMap.containsKey(r)) {
				c = rectangleColourMap.get(r);
			}
			g.setPaint(c);
			g.fill(r);
			g.setPaint(Color.BLACK);
			g.draw(r);
		}
		drawConstraints(g);

	}

	ArrayList<Rectangle2D> getRectangles() {
		if (graph != null) {
			return graph.getRectangles();
		}
		return rectangles;
	}

	private void setUpDrawingGraphics() {
		g = (Graphics2D) getGraphics();
		g.setColor(Color.black);
	}

	public void mousePressed(MouseEvent evt) {
		int x = evt.getX();
		int y = evt.getY();
		constraints = null;
		if (dragging == true)
			return;
		if (x > 0 && x < getWidth() && y > 0 && y < getHeight()) {
			prevX = x;
			prevY = y;
			dragging = true;
			setUpDrawingGraphics();
		}

	}

	public void drawConstraints(Graphics2D g) {
		if (constraints == null)
			return;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_OFF);
		Color originalColour = g.getColor();
		for (Constraint c : constraints) {
			c.colour = Color.BLUE;
			try {
				if (c.isViolated()) {
					c.colour = Color.RED;
					g.setColor(c.colour);
					Rectangle2D r1 = (Rectangle2D) c.left.data
							.get(Rectangle2D.class);
					Rectangle2D r2 = (Rectangle2D) c.right.data
							.get(Rectangle2D.class);
					// Chunk chunk = (Chunk)c.left.data.get(Chunk.class);
					g.drawLine((int) r1.getMinX(), (int) r1.getMinY(), (int) r2
							.getMinX(), (int) r2.getMinY());
				} else if (c.isTight()) {
					c.colour = Color.GREEN;
				}
			} catch (NullPointerException e) {
			}
		}
		g.setColor(originalColour);
	}

	public void mouseReleased(MouseEvent evt) {
		if (dragging == false)
			return;
		dragging = false;
		g.dispose();
		g = null;
		rectangles.add(rect);
		rect = null;
	}

	public void mouseDragged(MouseEvent evt) {
		if (dragging == false)
			return;
		int x = Math.min(evt.getX(), getSize().width - 1);
		x = Math.max(x, 0);
		int y = Math.min(evt.getY(), getSize().height - 1);
		y = Math.max(y, 0);
		paintComponent(g);
		Rectangle r = new Rectangle(Math.min(prevX, x), Math.min(prevY, y),
				Math.abs(x - prevX), Math.abs(y - prevY));
		g.drawRect(r.x, r.y, r.width, r.height);
		Color c = new Color(228, 228, 205);
		g.setPaint(c);
		g.fill(r);
		g.setPaint(Color.BLACK);
		g.draw(r);
		rect = r;
	}

	public void mouseEntered(MouseEvent evt) {
	}

	public void mouseExited(MouseEvent evt) {
	}

	public void mouseClicked(MouseEvent evt) {
	}

	public void mouseMoved(MouseEvent evt) {
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.util.Observer#update(java.util.Observable, java.lang.Object)
	 */
	public void update(Observable p, Object arg1) {
		constraints = ((QPRectanglePlacement) p).constraintGenerator.getConstraints();
		paintComponent(getGraphics());
	}

	public int print(Graphics g, PageFormat pf, int pi) throws PrinterException {
		if (pi >= 1) {
			return Printable.NO_SUCH_PAGE;
		}
		Graphics2D g2d = (Graphics2D) g;
		g2d.translate(pf.getImageableX(), pf.getImageableY());
		g2d.translate(pf.getImageableWidth() / 2, pf.getImageableHeight() / 2);
		Dimension d = new Dimension();
		for (Rectangle2D r : getRectangles()) {
			d.height = Math.max((int) r.getMaxY(), d.height);
			d.width = Math.max((int) r.getMaxX(), d.width);
		}
		double scale = Math.min(pf.getImageableWidth() / d.width, pf
				.getImageableHeight()
				/ d.height);
		if (scale < 1.0) {
			g2d.scale(scale, scale);
		}
		g2d.translate(-d.width / 2.0, -d.height / 2.0);
		render(d.width, d.height, g2d);
		return Printable.PAGE_EXISTS;
	}

	public void saveAsJPEG(String filename) {
		Dimension size = getSize();
		BufferedImage image = new BufferedImage(size.width, size.height,
				BufferedImage.TYPE_INT_RGB);
		Graphics2D g2 = image.createGraphics();
		paint(g2);
		try {
			OutputStream out = new BufferedOutputStream(new FileOutputStream(
					"c:/Temp/Test.png"));
			ImageIO.write(image, "png", out);
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
