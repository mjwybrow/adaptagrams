package placement;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
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
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
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
	enum InteractionMode {
		Create, Select
	};

	InteractionMode interactionMode = InteractionMode.Create;

	private int prevX, prevY;

	private boolean dragging;

	private Graphics2D g;

	private RectangleView rect;

	public ArrayList<RectangleView> rectangles = new ArrayList<RectangleView>();

	public Graph graph = null;

	private ArrayList<RectangleView> undoRectangles = new ArrayList<RectangleView>();

	private Constraints constraints;

	BlocksFileFilter fileFilter = new BlocksFileFilter();

	public void generateRandom(int n) {
		clear();
		Dimension dim = getSize();
		Random rand = new Random();
		double w = dim.width / 3.0;
		double h = dim.height / 3.0;
		generateRandom(n, rand, w, h);
	}

	protected void generateRandom(int n, Random rand, double w, double h) {
		for (int i = 0; i < n; i++) {
			RectangleView r = new RectangleView("" + i, w + rand.nextDouble()
					* w, h + rand.nextDouble() * h, rand.nextDouble()
					* w, rand.nextDouble() * h);
			rectangles.add(r);
		}
		int overlapCount = 0;
		for (int i = 0; i < rectangles.size(); i++) {
			RectangleView u = rectangles.get(i);
			Rectangle2D r1 = new Rectangle2D.Double(u.x, u.y, u.width, u.height);
			/*
			for (int j = i + 1; j < rectangles.size(); j++) {
				RectangleView v = rectangles.get(j);
				Rectangle2D r2 = new Rectangle2D.Double(v.x, v.y, v.width,
						v.height);
				if (r1.intersects(r2))
					overlapCount++;
			}
			*/
		}
		//System.out.println("Random graph has " + overlapCount + " overlaps.");
		repaint();
	}

	DrawingDimensions fitToScreen() {
		double xmax = 0;
		double xmin = Double.MAX_VALUE;
		double ymax = 0;
		double ymin = Double.MAX_VALUE;
		for (RectangleView r : rectangles) {
			xmin = Math.min(xmin, r.x);
			xmax = Math.max(xmax, (r.getMaxX()));
			ymin = Math.min(ymin, r.y);
			ymax = Math.max(ymax, (r.getMaxY()));
		}
		double currentWidth = Math.max(xmax, xmax - xmin);
		double currentHeight = Math.max(ymax, ymax - ymin);
		double targetWidth = getSize().width;
		double targetHeight = getSize().height;
		double xscale = targetWidth / currentWidth;
		double yscale = targetHeight / currentHeight;
		return new DrawingDimensions(Math.min(Math.min(xscale, yscale), 1),
				xmin, ymin);
	}

	protected void load(File f) {
		clear();
		ObjectInput input = null;
		if (f.getPath().endsWith(".dot")) {
			GraphParser g = new GraphParser(f.getPath());
			graph = g.getGraph();
			if (graph != null) {
				rectangles=graph.getRectangles();
			}
			fileFilter.lastSelectedFile = f;
		} else {
			try {
				// use buffering
				InputStream file = new FileInputStream(f);
				InputStream buffer = new BufferedInputStream(file);
				input = new ObjectInputStream(buffer);
				// deserialize the List
				rectangles = (ArrayList<RectangleView>) input.readObject();
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
		repaint();
	}

	protected File save(String path) {
		ObjectOutput output = null;
		if (!path.endsWith(".blocks")) {
			path = path + ".blocks";
		}
		File file = new File(path);
		try {
			// use buffering
			OutputStream buffer = new BufferedOutputStream(
					new FileOutputStream(file));
			output = new ObjectOutputStream(buffer);
			output.writeObject(rectangles);
		} catch (IOException ex) {
			ex.printStackTrace();
		} finally {
			try {
				if (output != null) {
					// flush and close "output" and its underlying
					// streams
					output.close();
				}
			} catch (IOException ex) {
				ex.printStackTrace();
			}
		}
		return file;
	}

	/**
	 * 
	 */
	protected void backup() {
		undoRectangles = new ArrayList<RectangleView>();
		for (RectangleView r : rectangles) {
			RectangleView nr = new RectangleView(r);
			undoRectangles.add(nr);
		}
	}

	protected void undo() {
		rectangles = undoRectangles;
		if(graph!=null){
			graph.restoreRectangles(rectangles);
		}
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
		rectangles = new ArrayList<RectangleView>();
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
		DrawingDimensions dim = fitToScreen();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		Color original = g.getColor();
		g.setColor(getBackground());
		g.fillRect(0, 0, width, height);
		g.setColor(original);
		super.paintChildren(g);
		if (graph != null) {
			for (Line2D l : graph.getLines()) {
				l.setLine(dim.toScreen(l.getP1()), dim.toScreen(l.getP2()));
				g.draw(l);
			}
		}
		for (RectangleView r : rectangles) {
			r.draw(g, dim);
		}
		drawConstraints(g);
	}

	private void setUpDrawingGraphics() {
		g = (Graphics2D) getGraphics();
		g.setColor(Color.black);
	}

	double selectOffsetX = 0;

	double selectOffsetY = 0;

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
			for (RectangleView r : rectangles) {
				Rectangle2D r2d = new Rectangle2D.Double(r.x, r.y, r.width,
						r.height);
				if (r2d.contains(x, y)) {
					rect = r;
				}
			}
			if (rect != null) {
				selectOffsetX = x - rect.x;
				selectOffsetY = y - rect.y;
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
					RectangleView r1 = (RectangleView) c.left.data
							.get(RectangleView.class);
					RectangleView r2 = (RectangleView) c.right.data
							.get(RectangleView.class);
					// Chunk chunk = (Chunk)c.left.data.get(Chunk.class);
					g.drawLine((int) r1.x, (int) r1.y, (int) r2.x, (int) r2.y);
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
				rect.colour = RectangleView.defaultRectColour;
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
				RectangleView r = new RectangleView("r" + rectangles.size(),
						Math.min(prevX, x), Math.min(prevY, y), Math.abs(x
								- prevX), Math.abs(y - prevY));
				r.draw(g, fitToScreen());
				rect = r;
				break;
			case Select:
				if (rect != null) {
					rect.moveTo(x - selectOffsetX, y - selectOffsetY);
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
		for (RectangleView r : rectangles) {
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

	public void setInteractionMode(InteractionMode interactionMode) {
		this.interactionMode = interactionMode;
	}

	public InteractionMode getInteractionMode() {
		return interactionMode;
	}
}
