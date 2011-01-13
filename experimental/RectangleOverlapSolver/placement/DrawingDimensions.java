/**
 * 
 */
package placement;

import java.awt.Point;
import java.awt.geom.Point2D;

class DrawingDimensions {

	public double scale;

	public double ymin;

	public double xmin;

	public DrawingDimensions(double scale, double xmin, double ymin) {
		this.scale = scale;
		this.xmin = xmin;
		this.ymin = ymin;
	}

	Point toScreen(double x, double y) {
		x -= xmin < 0 ? xmin : 0;
		y -= ymin < 0 ? ymin : 0;
		x *= scale;
		y *= scale;
		return new Point((int) x, (int) y);
	}

	public Point toScreen(Point2D p) {
		return toScreen(p.getX(), p.getY());
	}
}