module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
	
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;


// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                         mode;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;


wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;
wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////


// Detection variable and HSV value define
wire red_detect, green_detect, white_detect, yellow_detect, blue_detect;
reg [7:0] hue, saturation, value;
reg [7:0] max_val;
reg [7:0] min_val;
reg [7:0] delta;


// hsv translation  
always @(*) begin
  max_val = (red > green) ? ((red > blue) ? red : blue) : ((green > blue) ? green : blue);
  min_val = (red < green) ? ((red < blue) ? red : blue) : ((green < blue) ? green : blue);
  delta = max_val - min_val;
  if (delta == 0) begin
      hue = 0;
  end else if (max_val == red) begin
      hue = ((green < blue) ? (green - blue + 256) : (green - blue)) * 43 / delta;
  end else if (max_val == green) begin
      hue = ((blue < red ? (blue - red + 256) : (blue - red)) * 43 / delta) + 85;
  end else begin
      hue = ((red < green ? (red - green + 256) : (red - green)) * 43 / delta) + 171;
  end
// Compute saturation
   if (max_val != 0)
     saturation = (delta * 255) / max_val;
   else
     saturation = 0;
// Compute value
   value = max_val;
end
  
  
// Define HSV color ranges
parameter [7:0] RED_HUE_MIN = 0;       // Example range for red hue
parameter [7:0] RED_HUE_MAX = 25;
parameter [7:0] BLUE_HUE_MIN = 220;    // Example range for blue hue
parameter [7:0] BLUE_HUE_MAX = 250;
parameter [7:0] YELLOW_HUE_MIN = 30;   // Example range for yellow hue
parameter [7:0] YELLOW_HUE_MAX = 60;
parameter [7:0] SATURATION_MIN = 160;  // Example range for saturation
parameter [7:0] VALUE_MIN = 128;       // Example range for value

// Detect red areas
assign red_detect = (hue >= RED_HUE_MIN) & (hue <= RED_HUE_MAX) & (saturation >= SATURATION_MIN) & (value >= VALUE_MIN);
   
// Detect blue areas
assign blue_detect = (hue >= BLUE_HUE_MIN) & (hue <= BLUE_HUE_MAX) & (saturation >= SATURATION_MIN) & (value >= VALUE_MIN);
  
// Detect yellow areas
assign yellow_detect = (hue >= YELLOW_HUE_MIN) & (hue <= YELLOW_HUE_MAX) & (saturation >= SATURATION_MIN) & (value >= VALUE_MIN);

// Detect white areas using RGB
parameter [7:0] WHITE_MIN = 8'hFE, WHITE_MAX = 8'hFF;
assign white_detect = (red >= WHITE_MIN) & (red <= WHITE_MAX) & (green >= WHITE_MIN) & (green <= WHITE_MAX) & (blue >= WHITE_MIN) & (blue <= WHITE_MAX) & (saturation <= 40);

// Highlight detected areas of its red yellow or blue color
wire [23:0] highlight;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
assign highlight =(red_detect) ? {8'hff, 8'h00, 8'h00} :
                  (yellow_detect) ? {8'hff, 8'hff, 8'h00} :
                  (blue_detect) ? {8'h00, 8'h00, 8'hff} :
						(white_detect) ? {8'hff, 8'hff, 8'hff}:
                  {grey, grey, grey};

// Show bounding box
wire [23:0] new_image;
wire bbr_active, bbb_active, bby_active, bbw_active; // active is 1 when meeting red, blue and yellow bounding boxes
assign bbr_active = (x == r_left) | (x == r_right) | (y == r_top) | (y == r_bottom);  // assign active to 1 when there is a bounding box
assign bbb_active = (x == b_left) | (x == b_right) | (y == b_top) | (y == b_bottom);
assign bby_active = (x == y_left) | (x == y_right) | (y == y_top) | (y == y_bottom);
assign new_image = (bbr_active) ? {8'hff, 8'h00, 8'h00} : // new imagesshow the bounding boxes for different color while keeping the remain no change
                   (bbb_active) ? {8'h00, 8'h00, 8'hff} : 
                   (bby_active) ? {8'hff, 8'hff, 8'h00} : 
                   highlight;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end


// add counter for counting whether there is enough pixels of a color to trigger the bounding box 
// detected is 1 when there is a red, yellow, blue or white pixel detected
// define counters to count the number of colored pixel
// all counter values reset every 16 pixels
wire detected; 
assign detected = red_detect | yellow_detect | blue_detect | white_detect;
reg [3:0] c_counter, r_counter, b_counter, y_counter, w_counter, counter;
always@(posedge clk) begin
  c_counter <= c_counter + 4'b1;
  if (red_detect & !white_detect) r_counter <= r_counter + 4'b1;
  if (yellow_detect & !white_detect) y_counter <= y_counter + 4'b1;
  if (blue_detect & !white_detect) b_counter <= b_counter + 4'b1;
  if (white_detect) w_counter <= w_counter + 4'b1;
  if (detected) counter <= counter + 4'b1;
  if (c_counter == 4'b0) begin
      r_counter <= 4'b0;
		y_counter <= 4'b0;
		b_counter <= 4'b0;
		w_counter <= 4'b0;
		counter <= 4'b0;
	end
end

//Find first and last red pixels
reg [10:0] x_rmin, y_rmin, x_rmax, y_rmax;
always@(posedge clk) begin
	//Update bounds when there is enough red pixels in 16 pixels
	if (red_detect & !white_detect & in_valid & (r_counter > 4'hc | ((4'hf - counter > 4'h8)&r_counter > 4'h5))) begin
		if (x < x_rmin) x_rmin <= x;
		if (x > x_rmax) x_rmax <= x;
		if (y < y_rmin) y_rmin <= y;
		y_rmax <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_rmin <= IMAGE_W-11'h1;
		x_rmax <= 0;
		y_rmin <= IMAGE_H-11'h1;
		y_rmax <= 0;
	end
end

//Find first and last blue pixels
reg [10:0] x_bmin, y_bmin, x_bmax, y_bmax;
always@(posedge clk) begin
	if (blue_detect & !white_detect & in_valid & (b_counter > 4'hc | ((4'hf - counter > 4'h8)&b_counter > 4'h5))) begin	//Update bounds when the pixel is blue
		if (x < x_bmin) x_bmin <= x;
		if (x > x_bmax) x_bmax <= x;
		if (y < y_bmin) y_bmin <= y;
		y_bmax <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_bmin <= IMAGE_W-11'h1;
		x_bmax <= 0;
		y_bmin <= IMAGE_H-11'h1;
		y_bmax <= 0;
	end
end

//Find first and last yellow pixels
reg [10:0] x_ymin, y_ymin, x_ymax, y_ymax;
always@(posedge clk) begin
	if (yellow_detect & !white_detect & in_valid & (y_counter > 4'hc | ((4'hf - counter > 4'h8)&y_counter > 4'h5))) begin //Update bounds when the pixel is yellow
		if (x < x_ymin) x_ymin <= x;
		if (x > x_ymax) x_ymax <= x;
		if (y < y_ymin) y_ymin <= y;
		y_ymax <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_ymin <= IMAGE_W-11'h1;
		x_ymax <= 0;
		y_ymin <= IMAGE_H-11'h1;
		y_ymax <= 0;
	end
end

//Find if there is a white wall in front of the rover
reg white_wall_infront;
always@(posedge clk) begin
   //Update bounds when there is enough white pixels in 16 pixels
   if (white_detect & in_valid & (w_counter > 4'hc | ((4'hf - counter > 4'h8)&w_counter > 4'h5)))begin	//Update bounds when the pixel is white
      // white_wall_infront is 1 when there is a pixel in front of the rover in a certain area
		if (y < 11'd480 && y > 11'd300 && x < 11'd430 && x > 11'd210) white_wall_infront = 1'b1;
	end
	if (sop & in_valid) begin	//Reset white_wall_infront on start of packet
      white_wall_infront = 1'b0;
	end
end

//Process bounding box at the end of the frame.
//Define bounding lines for red, blue, and yellow color
reg [1:0] msg_state;
reg [10:0] r_left, r_right, r_top, r_bottom;
reg [10:0] b_left, b_right, b_top, b_bottom;
reg [10:0] y_left, y_right, y_top, y_bottom;
reg [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		r_left <= x_rmin;
		r_right <= x_rmax;
		r_top <= y_rmin;
		r_bottom <= y_rmax;
		
		b_left <= x_bmin;
		b_right <= x_bmax;
		b_top <= y_bmin;
		b_bottom <= y_bmax;
		
		y_left <= x_ymin;
		y_right <= x_ymax;
		y_top <= y_ymin;
		y_bottom <= y_ymax;
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 2'b01;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 2'b00) begin
	  if (msg_state == 2'b10) msg_state <= msg_state + 2'b10;
	  else msg_state <= msg_state + 2'b01;
	end
end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;
reg [1:0] det; 

`define RED_BOX_MSG_ID "RBB"
`define BLUE_BOX_MSG_ID "BBB"
`define YELLOW_BOX_MSG_ID "YBB"
`define WHITE_WALL_MSG_ID "WWM"

// there is 4 states
// 01 states generate the ID of the message
// 10 states generate the boolean information for beacons and white walls
always@(posedge clk) begin //Write words to FIFO as state machine advances
 case(msg_state)
  2'b00: begin
   msg_buf_in = 32'b0;
   msg_buf_wr = 1'b0;
  end
  2'b01: begin
    case(det)
    2'b00: msg_buf_in = `RED_BOX_MSG_ID;
    2'b01: msg_buf_in = `BLUE_BOX_MSG_ID;
    2'b10: msg_buf_in = `YELLOW_BOX_MSG_ID;
    2'b11: msg_buf_in = `WHITE_WALL_MSG_ID;
    endcase
   msg_buf_wr = 1'b1;
  end
  2'b10: begin
   case(det)
    2'b00:   if (x_rmax > 11'd300 && x_rmin < 11'd340) msg_buf_in = 1'b1;
	          else msg_buf_in = 1'b0;
    2'b01:   if (x_bmax > 11'd300 && x_bmin < 11'd340) msg_buf_in = 1'b1;
	          else msg_buf_in = 1'b0;
    2'b10:   if (x_ymax > 11'd300 && x_ymin < 11'd340) msg_buf_in = 1'b1;
	          else msg_buf_in = 1'b0;
    2'b11:   msg_buf_in = white_wall_infront;
   endcase
	case(det)
	// det shifts after transmitting one beacon data
    2'b00: det = 2'b01;
    2'b01: det = 2'b10;
    2'b10: det = 2'b11;
    2'b11: det = 2'b00;
   endcase
   msg_buf_wr = 1'b1;
  end
  default: ;
 endcase
end


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
	);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule

