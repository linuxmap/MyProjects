// ArchiveBuffer.h

#ifndef _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_
#define _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_

#include <streambuf>
#include <stdexcept>

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class ArchiveBuffer
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::int_type int_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::traits_type traits_type;

            ArchiveBuffer(
                char_type * buf, 
                std::size_t buf_size, 
                std::size_t data_size = 0)
                : buf_(buf)
                , buf_size_(buf_size)
            {
                this->setg(buf_, buf, buf + data_size);
                this->setp(buf_, buf + buf_size);
                this->pbump(data_size);
            }

            /**
            返回读缓存的数据

            并不移动读指针。
            */
            boost::asio::const_buffers_1 data(
                std::size_t n = std::size_t(-1))
            {
                if (this->gptr() + n > this->pptr())
                    n = this->pptr() - this->gptr();
                return boost::asio::const_buffers_1(this->gptr(), n);
            }

            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            /**
            将写缓存的数据提交的读缓存

            追加从写指针开始的 @c n 字节数据到读缓存，写指针向前移动 @c n 字节。
            需要在外部写入实际数据，然后提交
             */
            void commit(
                std::size_t n)
            {
                if (this->pptr() + n > this->epptr())
                    n = this->epptr() - this->pptr();
                this->pbump(static_cast<int>(n));
                this->setg(this->eback(), this->gptr(), this->pptr());
            }

            /**
            从读缓存移除数据

            读指针向前移动 @c n 字节。
            需要在外部读出实际数据，然后移除相应数据
            */
            void consume(
                std::size_t n)
            {
                if (this->gptr() + n > this->pptr())
                    n = this->pptr() - this->gptr();
                this->gbump(static_cast<int>(n));
                this->setg(this->eback(), this->gptr(), this->pptr());
            }

            /**
            在写缓存中准备一段空间

            并不移动写指针。
            */
            boost::asio::mutable_buffers_1 prepare(std::size_t n)
            {
                if (this->pptr() + n > this->epptr())
                    n = this->epptr() - this->pptr();
                return boost::asio::mutable_buffers_1(this->pptr(), n);
            }

        protected:
            virtual int_type underflow()
            {
                return traits_type::eof();
            }

            virtual int_type overflow(int_type c)
            {
                throw std::length_error("archive buffer too long");
                return traits_type::not_eof(c);
            }

        private:
            char_type * buf_; ///< 缓存首地址
            std::size_t buf_size_; ///< 缓存大小

        }; // class ArchiveBuffer

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_
