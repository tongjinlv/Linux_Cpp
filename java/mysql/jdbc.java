import java.sql.Connection;  
import java.sql.DriverManager;  
import java.sql.ResultSet;  
import java.sql.SQLException;  
import java.sql.Statement;  
  
  
public class jdbc {  
    public static void main(String[] args) {  
        Connection conn = null;  
        try {  
            Class.forName("org.gjt.mm.mysql.Driver");  
        } catch (ClassNotFoundException e) {  
            System.out.println("无法加载驱动");  
        }  
        try {  
            conn = DriverManager.getConnection("jdbc:mysql://localhost:3306/books", "root", "zq19890319");  
        } catch (SQLException e) {  
            System.out.println("无法连接数据库");  
        }  
        try {  
            Statement stmt = conn.createStatement();  
            ResultSet rs = stmt.executeQuery("select * from orders");  
            while(rs.next()) {  
                int x = rs.getInt("orders_id");  
                System.out.println(x);  
            }  
        } catch (SQLException e) {  
            System.out.println("无法查询");  
        }  
          
    }  
}  

