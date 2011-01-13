package placement;

import java.awt.Rectangle;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class GraphParser {
	Graph g = new Graph();

	GraphParser(String fileName) {
		Pattern nodePattern = Pattern.compile("^\\s*(\\w+)\\s*\\[(.*)]");
		Pattern labelPattern = Pattern.compile("label\\s*=\\s*\"?(\\w+)");
		Pattern widthPattern = Pattern
				.compile("width\\s*=\\s*\"?(\\d+(\\.\\d+)?)");
		Pattern heightPattern = Pattern
				.compile("height\\s*=\\s*\"?(\\d+(\\.\\d+)?)");
		Pattern posPattern = Pattern.compile("pos\\s*=\\s*\"(-?\\d+),(-?\\d+)!?");
		Pattern edgePattern = Pattern
				.compile("^\\s*(\\w+)\\s*(->|--)\\s*(\\w+)");
		try {
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String s;
			while ((s = br.readLine()) != null) {
				Matcher m = nodePattern.matcher(s);
				if (m.find()) {
					String id = m.group(1);
					if (!id.equalsIgnoreCase("node")
							&& !id.equalsIgnoreCase("graph")) {
						String attrs = m.group(2);
						String label = match(labelPattern, attrs);
						String widthStr = match(widthPattern, attrs);
						String heightStr = match(heightPattern, attrs);
						int width = widthStr == null ? label.length() * 12
								: (int) (72f * Float.parseFloat(widthStr));
						int height = heightStr == null ? 16
								: (int) (72f * Float.parseFloat(match(
								heightPattern, attrs)));
						int x = Integer.parseInt(match(posPattern, attrs));
						int y = Integer.parseInt(match(posPattern, attrs, 2));
						System.out.printf("Node: %s %s %d %d %d %d\n", id,
								label, width, height, x, y);
						g.addNode(id, x, -y, width, height);
					}
				}
				m = edgePattern.matcher(s);
				if (m.find()) {
					g.addEdge(m.group(1), m.group(3));
				}
			}
		} catch (FileNotFoundException fnfe) {
			System.out
					.println("Cannot locate input file! " + fnfe.getMessage());
			System.exit(0);
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	private String match(Pattern p, String s) {
		return match(p, s, 1);
	}

	private String match(Pattern p, String s, int group) {
		Matcher m = p.matcher(s);
		String val = null;
		if (m.find()) {
			val = m.group(group);
		}
		return val;
	}

	public Graph getGraph() {
		return g;
	}
}
