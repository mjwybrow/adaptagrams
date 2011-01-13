package placement;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.Serializable;

public class RectangleView implements Serializable {
	private static final long serialVersionUID = 3617292346069431353L;

	double x, y, width, height, oldX;

	String label;

	Color colour = defaultRectColour;

	static final Color defaultRectColour = new Color(228, 228, 205, 200);

	RectangleView(String label, double x, double y, double w, double h) {
		this.label = label;
		this.x = x;
		this.y = y;
		width = w;
		height = h;
	}

	public RectangleView(RectangleView r) {
		this(new String(r.label), r.x, r.y, r.width, r.height);
		colour = r.colour;
	}

	public void draw(Graphics2D g, DrawingDimensions dim) {
		Point p = dim.toScreen(x, y);
		double w = width * dim.scale;
		double h = height * dim.scale;
		draw(g, p, w, h);
	}

	public void draw(Graphics2D g, int x, int y, int w, int h) {
		draw(g, new Point(x, y), w, h);
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
		this.x = x;
		this.y = y;
	}

	public double getMaxY() {
		return y+height;
	}

	public double getMaxX() {
		return x+width;
	}

}
