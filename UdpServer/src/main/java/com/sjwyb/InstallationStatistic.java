package com.sjwyb;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class InstallationStatistic extends AbstractLogStatistic {
	private final Map<String, Integer> monthMapping;
	private Map<String, Long> ipCounter;
	
	public InstallationStatistic() {
		monthMapping = new HashMap<String, Integer>(12);
		monthMapping.put("Jan", 0);
	    monthMapping.put("Feb", 1);
	    monthMapping.put("Mar", 2);
	    monthMapping.put("Apr", 3);
	    monthMapping.put("May", 4);
	    monthMapping.put("Jun", 5);
	    monthMapping.put("Jul", 6);
	    monthMapping.put("Aug", 7);
	    monthMapping.put("Sep", 8);
	    monthMapping.put("Oct", 9);
	    monthMapping.put("Nov", 10);
	    monthMapping.put("Dec", 11);
	    
		ipCounter = new HashMap<String, Long>();
	}
	
	/*
	 * line example:
	 * 
	 * 123.179.43.157 - - [24/Aug/2013:18:19:49 +0800] \
	 * "GET /fuzhu/sjwybpre.html?re=Install%20Gauss%20successfully \
	 * HTTP/1.1" 200 30
	 */
	
	@Override
	long parseTimeStamp(String line) {
		String timestr = line.split("\\s+")[3].substring(1);
		String[] part1 = timestr.split("/");
		int date = Integer.parseInt(part1[0]);
		int month = monthMapping.get(part1[1]);
		String[] part2 = part1[2].split(":");
		int year = Integer.parseInt(part2[0]);
		int hourOfDay = Integer.parseInt(part2[1]);
		int minute = Integer.parseInt(part2[2]);
		int second = Integer.parseInt(part2[3]);
		Calendar calendar = Calendar.getInstance();
		calendar.set(year, month, date, hourOfDay, minute, second);
		return calendar.getTimeInMillis();
	}

	@Override
	boolean incrementCounter(String line) {
		String ip = line.split("\\s+")[0];
		Long counter = ipCounter.get(ip);
		if (counter == null)
			counter = new Long(0);
		ipCounter.put(ip, new Long(counter + 1));
		return counter == 0;
	}
	
	public void outputResult() throws IOException {
		handleLogRecord();
		
		System.out.printf(
			"\nInstallation Count\n"+
			"==================\n" +
			"       Today: %d\n" +
			"Last 3  days: %d\n" +
			"Last 7  days: %d\n" +
			"Last 15 days: %d\n" +
			"Last 30 days: %d\n",
			counters[TODAY],
			counters[LAST_3_DAYS],
			counters[LAST_7_DAYS], 
			counters[LAST_15_DAYS],
			counters[LAST_30_DAYS]);
		
		if (ipCounter.size() < 5)
			return;
		
		List<Map.Entry<String, Long>> list = new ArrayList<Map.Entry<String,Long>>(
				ipCounter.entrySet());
		Collections.sort(list, new Comparator<Map.Entry<String, Long>>() {
			public int compare(Map.Entry<String, Long> e1, Map.Entry<String, Long> e2) {
				long counter1 = e1.getValue();
				long counter2 = e2.getValue();
				return counter1 < counter2 ? 1 : (counter1 == counter2 ? 0 : -1);
			};
		});
		System.out.printf("\nTop 5 common ip\n");
		System.out.printf("===============\n");
		for (int i = 0; i < 5; i++) {
			System.out.printf("%s(%d)\n", list.get(i).getKey(), list.get(i).getValue());
		}
	}

	public static void main(String[] args) throws IOException {
		new InstallationStatistic().outputResult();
	}

}
