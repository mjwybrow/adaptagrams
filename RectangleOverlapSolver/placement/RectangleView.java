package placement;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.Rectangle2D;

class RectangleView {
	Rectangle2D.Double r;
	double width, height;
	String label;

	Color colour = defaultRectColour;

	static final Color defaultRectColour = new Color(228, 228, 205, 200);

	RectangleView(String label, double x, double y, double w, double h) {
		this(label,new Rectangle2D.Double(x, y, w, h));
	}

	public RectangleView(String rectangleLabel, Rectangle2D.Double rect) {
		label = rectangleLabel;
		r = rect;
		width=r.getWidth(); height=r.getHeight();
	}

	public RectangleView(RectangleView r2) {
		this(new String(r2.label),(Rectangle2D.Double) r2.r.clone());
		colour = r2.colour;
	}

	public void draw(Graphics2D g, DrawingDimensions dim) {
		Point p = dim.toScreen(r.getMinX(), r.getMinY());
		double w = width * dim.scale;
		double h = height * dim.scale;
		draw(g, p, w, h);
	}

	public void draw(Graphics2D g, int x, int y, int w, int h) {
		draw(g, new Point(x,y), w, h);
	}
	
	private void draw(Graphics2D g, Point p, double w, double h) {
		Rectangle vr = new Rectangle(p.x, p.y, (int) w, (int) h);
		g.setPaint(colour);
		g.fill(vr);
		g.setPaint(Color.BLACK);
		g.draw(vr);
		Font f = new Font("Times New Roman", Font.PLAIN, 24);
		FontMetrics fm = g.getFontMetrics(f);
		double fh = fm.getHeight();
		double fw = fm.stringWidth(label);
		int d = fm.getMaxDescent();
		double fsize = 0.9 * 24.0 * Math.min(w / fw, h / fh);
		f = new Font("Times New Roman", Font.PLAIN, (int) fsize);
		g.setFont(f);
		fm = g.getFontMetrics(f);
		fh = fm.getHeight();
		fw = fm.stringWidth(label);
		g.drawString(label, p.x, p.y + (int) (fh / 1.3));
	}
	public void moveTo(double x, double y) {
		r.setRect(x, y, width, height);
	}

}
