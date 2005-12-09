package placement;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
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

class PRect {
	public static double scale, xmin, ymin;

	Rectangle2D r;

	String label;

	Color colour = defaultRectColour;

	static final Color defaultRectColour = new Color(228, 228, 205, 200);

	PRect(String label, double x, double y, double w, double h) {
		this.label = label;
		r = new Rectangle2D.Double(x, y, w, h);
	}

	public PRect(String rectangleLabel, Rectangle2D rect) {
		label = rectangleLabel;
		r = rect;
	}

	public PRect(PRect r2) {
		r = (Rectangle2D) r2.r.clone();
		label = new String(r2.label);
		colour = r2.colour;
	}

	public void draw(Graphics2D g) {
		double x = r.getMinX();
		double y = r.getMinY();
		double w = r.getWidth();
		double h = r.getHeight();
		x -= xmin < 0 ? xmin : 0;
		y -= ymin < 0 ? ymin : 0;
		x *= scale;
		w *= scale;
		y *= scale;
		h *= scale;
		Rectangle2D vr = new Rectangle2D.Double(Math.floor(x), Math.floor(y),
				Math.floor(w), Math.floor(h));

		g.setPaint(colour);
		g.fill(vr);
		g.setPaint(Color.BLACK);
		g.draw(vr);
		Font f = new Font("Times New Roman", Font.PLAIN, 24);
		FontMetrics fm = g.getFontMetrics(f);
		int fh = fm.getHeight();
		int fw = fm.stringWidth(label);
		int d = fm.getMaxDescent();
		double fsize = 0.9 * 24.0 * Math.min(w / (double) fw, h / (double) fh);
		f = new Font("Times New Roman", Font.PLAIN, (int) fsize);
		g.setFont(f);
		fm = g.getFontMetrics(f);
		fh = fm.getHeight();
		fw = fm.stringWidth(label);
		g.drawString(label, (int) x, (int) (y + fh / 1.3));
	}
}

public class RectangleDrawerPanel extends JPanel implements Printable,
		MouseInputListener, Observer {
	enum InteractionMode {
		Create, Select
	};

	InteractionMode interactionMode = InteractionMode.Create;

	private int prevX, prevY;

	private boolean dragging;

	private Graphics2D g;

	private PRect rect;

	public ArrayList<PRect> rectangles = new ArrayList<PRect>();

	public Graph graph = null;

	private ArrayList<PRect> undoRectangles = new ArrayList<PRect>();

	private Constraints constraints;

	BlocksFileFilter fileFilter = new BlocksFileFilter();

	protected void generateRandom() {
		clear();
		Dimension dim = getSize();
		int n = 50;
		Random rand = new Random();
		double w = dim.width / 3.0;
		double h = dim.height / 3.0;
		for (int i = 0; i < n; i++) {
			PRect r = new PRect("" + i, w + rand.nextDouble() * w, h
					+ rand.nextDouble() * h, rand.nextDouble() * (w / 3.0),
					rand.nextDouble() * (h / 3.0));
			rectangles.add(r);
		}
		int overlapCount = 0;
		for (int i = 0; i < rectangles.size(); i++) {
			Rectangle2D u = rectangles.get(i).r;
			for (int j = i + 1; j < rectangles.size(); j++) {
				Rectangle2D v = rectangles.get(j).r;
				if (u.intersects(v))
					overlapCount++;
			}
		}
		System.out.println("Random graph has " + overlapCount + " overlaps.");
		repaint();
	}

	void fitToScreen() {
		double xmax = 0;
		double xmin = Double.MAX_VALUE;
		double ymax = 0;
		double ymin = Double.MAX_VALUE;
		for (PRect r : rectangles) {
			xmin = Math.min(xmin, r.r.getMinX());
			xmax = Math.max(xmax, (r.r.getMinX() + r.r.getWidth()));
			ymin = Math.min(ymin, r.r.getMinY());
			ymax = Math.max(ymax, (r.r.getMinY() + r.r.getHeight()));
		}
		double currentWidth = Math.max(xmax, xmax - xmin);
		double currentHeight = Math.max(ymax, ymax - ymin);
		double targetWidth = getSize().width;
		double targetHeight = getSize().height;
		double xscale = targetWidth / currentWidth;
		double yscale = targetHeight / currentHeight;

		PRect.scale=Math.min(xscale,yscale);
		PRect.scale = Math.min(PRect.scale,1);
		PRect.xmin = xmin;
		PRect.ymin = ymin;
	}

	protected void load(File f) {
		clear();
		ObjectInput input = null;
		if (f.getPath().endsWith(".dot")) {
			GraphParser g = new GraphParser(f.getPath());
			graph = g.getGraph();
			if (graph != null) {
				for (Rectangle2D rect : graph.getRectangles()) {
					rectangles.add(new PRect(graph.getRectangleLabel(rect),
							rect));
				}
			}
			for (Rectangle2D l : idMap.keySet()) {
				System.out.println("in load: " + idMap.get(l));
			}
			fileFilter.lastSelectedFile = f;
		} else {
			try {
				// use buffering
				InputStream file = new FileInputStream(f);
				InputStream buffer = new BufferedInputStream(file);
				input = new ObjectInputStream(buffer);
				// deserialize the List
				rectangles = (ArrayList<PRect>) input.readObject();
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
		undoRectangles = new ArrayList<PRect>();
		for (PRect r : rectangles) {
			PRect nr = new PRect(r);
			undoRectangles.add(nr);
		}
	}

	protected void undo() {
		rectangles = undoRectangles;
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
		rectangles = new ArrayList<PRect>();
		constraints = null;
		graph = null;
		repaint();
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

			Graphics2D offscreenGraphics = (Graphics2D) volatileImg
					.getGraphics();
			render(getWidth(), getHeight(), offscreenGraphics); // call core
			// paint method.

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

		fitToScreen();
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
		for (PRect r : rectangles) {
			r.draw(g);
			String label = idMap.get(r);
		}
		drawConstraints(g);

	}

	private void setUpDrawingGraphics() {
		g = (Graphics2D) getGraphics();
		g.setColor(Color.black);
	}

	double selectOffsetX = 0;

	double selectOffsetY = 0;

	public Hashtable<Rectangle2D, String> idMap = new Hashtable<Rectangle2D, String>();

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
		if (interactionMode == InteractionMode.Select) {
			rect = null;
			for (PRect r : rectangles) {
				if (r.r.contains(x, y)) {
					rect = r;
				}
			}
			if(rect!=null) {
				selectOffsetX = x - rect.r.getMinX();
				selectOffsetY = y - rect.r.getMinY();
				rect.colour = Color.PINK;
			}
			paintComponent(g);
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
		switch (interactionMode) {
		case Create:
			if (dragging == false)
				return;
			g.dispose();
			g = null;
			if (rect != null)
				rectangles.add(rect);
			rect = null;
			break;
		case Select:
			if (rect != null) {
				rect.colour = PRect.defaultRectColour;
				paintComponent(g);
				rect = null;
			}
			break;
		}
		dragging = false;
	}

	public void mouseDragged(MouseEvent evt) {
		if (dragging == false)
			return;
		int x = Math.min(evt.getX(), getSize().width - 1);
		x = Math.max(x, 0);
		int y = Math.min(evt.getY(), getSize().height - 1);
		y = Math.max(y, 0);
		paintComponent(g);
		if (prevX != x && prevY != y) {
			switch (interactionMode) {
			case Create:
				PRect r = new PRect("r"+rectangles.size(), Math.min(prevX, x), Math
						.min(prevY, y), Math.abs(x - prevX), Math
						.abs(y - prevY));
				r.draw(g);
				rect = r;
				break;
			case Select:
				if (rect != null) {
					rect.r.setRect(x - selectOffsetX, y - selectOffsetY, rect.r
							.getWidth(), rect.r.getHeight());
				}
				break;
			}
		}
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
		constraints = ((QPRectanglePlacement) p).constraintGenerator
				.getConstraints();
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
		for (PRect r : rectangles) {
			d.height = Math.max((int) r.r.getMaxY(), d.height);
			d.width = Math.max((int) r.r.getMaxX(), d.width);
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

	public void setInteractionMode(InteractionMode interactionMode) {
		this.interactionMode = interactionMode;
	}

	public InteractionMode getInteractionMode() {
		return interactionMode;
	}

	public ArrayList<Rectangle2D> getRectangles() {
		ArrayList<Rectangle2D> rs = new ArrayList<Rectangle2D>();
		for (PRect r : rectangles) {
			rs.add(r.r);
		}
		return rs;
	}
}
