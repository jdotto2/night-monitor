FROM balenalib/raspberrypi4-64-debian-python:3.11.5

WORKDIR /night-monitor

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

CMD ["sh", "start.sh"]